<?php
/**
 * @file
 * This file contains the functions used for administration of the module
 *
 */

/*no effect?
function tripal_trait_admin_trait_view() {
  $output = '';
  
  // set the breadcrumb
  $breadcrumb = array();
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('Tripal', 'admin/tripal');
  $breadcrumb[] = l('Extensions', 'admin/tripal/extensions');
  $breadcrumb[] = l('Trait', 'admin/tripal/chado/tripal_trait');
  drupal_set_breadcrumb($breadcrumb);
  
  return $output;
}
*/

function tripal_trait_admin_listing() {
  $output = '';

  // set the breadcrumb
  $breadcrumb = array();
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('Tripal', 'admin/tripal');
  $breadcrumb[] = l('Extensions', 'admin/tripal/extensions');
  $breadcrumb[] = l('Trait', 'admin/tripal/chado/tripal_trait');
  drupal_set_breadcrumb($breadcrumb);


  $output .= "<p>The Tripal Trait Module provides functionality for traits, maps, and markers.</p>";

  return $output;
}

/**
 * Administrative settings form
 *
 * @ingroup tripal_trait
 */
function tripal_trait_admin() {
  $form = array();

  $form['general'] = array(
    '#type' => 'fieldset',
    '#title' => 'General'
  );

  $form['general']['display_empty']= array(
    '#type' => 'checkbox',
    '#title' => t('Display fields if empty'),
    '#description' => t('Some websites may have a policy of showing all possible fields, or of never showing a field label that has no content.'),
    '#default_value' => variable_get('display_empty', true),
  );

  $form['general']['use_markers']= array(
    '#type' => 'checkbox',
    '#title' => t('Use marker pages'),
    '#description' => t('Marker pages can be disabled if a website already has custom support for markers.'),
    '#default_value' => variable_get('use_markers', false),
  );

  return system_settings_form($form);
}


function tripal_trait_load_data() {
//drupal_set_message('tripal_trait_load_data');
  
  $form = array();

  $form['instructions'] = array(
    '#type' => 'item',
    '#markup' => theme('tripal_trait_load'),
  );

  // Upload files
  $form['#attributes']['enctype'] = 'multipart/form-data';
  $form['upload'] = array(
    '#type' => 'fieldset',
    '#title' => 'Upload files',
  );
  $field_separators = array('comma' => t('comma'), 'tab' => t('tab'));
  $form['upload']['deliminator'] = array(
    '#type' => 'select',
    '#title' => 'Field separator',
    '#options' => $field_separators,
    '#default' => 0,
  );
    
  $file_uploads = tripal_trait_get_file_uploads();
  foreach ($file_uploads as $file_upload) {
    $form['upload'][$file_upload['element']]= array(
      '#type' => 'file',
      '#title' => t($file_upload['label']),
    );
  }

  // Verify data
  $form['verify'] = array(
    '#type' => 'fieldset',
    '#title' => t('Verify Data'),
    '#description' => t('It may save time to first verify all files before loading.'),
  );
  $form['verify']['verify_submit'] = array(
    '#type' => 'submit',
    '#title' => t('Verify'),
    '#value' => t('Verify'),
  );
  
  // Load data
  $html = "Load your data.";
  $form['load'] = array(
    '#type' => 'fieldset',
    '#title' => t('Load Data'),
    '#description' => t($html),
  );
  $form['load']['load_submit'] = array(
    '#type' => 'submit',
    '#title' => t('Load'),
    '#value' => t('Load'),
  );

  // Working space
  $form['work'] = array(
    '#type' => 'item',
    '#markup' => t('<div name="workdiv" id="workdiv"></div>'),
  );
  
  return $form;
}

/**
 *
 * @ingroup tripal_trait
 */
function tripal_trait_load_data_validate($form, &$form_state) {
//drupal_set_message('tripal_trait_load_data_validate', 'status');
  $job_id = "HVmVDT";//getUniqueID(6);
  $_SESSION['trait_job_id'] = $job_id;
  
  $upload_ready = false;
  $file_uploads = tripal_trait_get_file_uploads();
//drupal_set_message("Got " . count($file_uploads) . " uploaded files.");

  foreach ($file_uploads as $file_upload) {
    $element = $file_upload['element'];
    if (isset($form['upload'][$element])) {
      $fh = file_save_upload($element, array());
      if ($fh) {
        $tmp_filename = "temporary://$job_id.$element";
        file_move($fh, $tmp_filename, FILE_EXISTS_REPLACE);//FILE_EXISTS_RENAME);
//drupal_set_message("Put uploaded file into $tmp_filename");
        $upload_ready = true;
      }
      else {
        drupal_set_message("Failed to upload $element: error=" . $_FILES['files']['error'][$file_upload['element']]);
      }
    }
  }

  if (!$upload_ready) {
    drupal_set_message("No files uploaded");
    form_set_error('No data files have been uploaded', 'error');
  }

//drupal_set_message('tripal_trait_load_data_validate() completed', 'status');
}

/**
 *
 * @ingroup tripal_trait
 */
function tripal_trait_load_data_submit($form, &$form_state) {
//drupal_set_message("tripal_trait_load_data_submit", 'status');
  $job_id = $_SESSION['trait_job_id'];
  
  $job_args = array(
    'job_id' => $job_id
  );
  $tripal_job_id = tripal_add_job(
      t("Load QTL/Marker/Map data [$job_id]"),
      'tripal_trait',
      'tripal_trait_run_data_loaders',
      $job_args,
      $user->uid
    );
drupal_set_message("tripal_trait_load_data_submit(): job ID = $job_id, tripal job id = $tripal_job_id", 'status');

    tripal_jobs_launch(1, $tripal_job_id);

  // Go wait for the results
  drupal_goto('admin/tripal/extension/tripal_trait/upload_results');
}

/**
 *
 * @ingroup tripal_trait
 */
function tripal_trait_admin_upload_results() {
  $job_id = $_SESSION['trait_job_id'];
  

  return theme('tripal_trait_upload_results');
}

/**
 *
 * @ingroup tripal_trait
 */
function tripal_trait_get_file_uploads() {
  return array(
/*
    array(
      'element' => 'upload_contact',
      'label' => 'Upload Contact worksheet',
    ),
    array(
      'element' => 'upload_publication',
      'label' => 'Upload Publication worksheet',
    ),
    array(
      'element' => 'upload_site-environment',
      'label' => 'Upload Site-Environment worksheet',
    ),
    array(
      'element' => 'upload_dataset',
      'label' => 'Upload Dataset worksheet',
    ),
    array(
      'element' => 'upload_stock',
      'label' => 'Upload Stock worksheet',
    ),
    array(
      'element' => 'upload_mapset',
      'label' => 'Upload MapSet worksheet',
    ),
    array(
      'element' => 'upload_linkagegroup',
      'label' => 'Upload LinkageGroup worksheet',
    ),
    array(
      'element' => 'upload_qtl',
      'label' => 'Upload QTL worksheet',
    ),
    array(
      'element' => 'upload_qtl_trait',
      'label' => 'Upload QTLTrait worksheet',
    ),
    array(
      'element' => 'upload_marker',
      'label' => 'Upload Marker worksheet',
    ),
    array(
      'element' => 'upload_mapposition',
      'label' => 'Upload MapPosition worksheet',
    ),
    array(
      'element' => 'upload_image',
      'label' => 'Upload Image worksheet',
    ),
*/
    array(
      'element' => 'upload_file',
      'label' => 'Upload a worksheet converted to a CSV file.',
    ),
  );
}

/**
 *
 * @ingroup tripal_trait
 */
function getUniqueID($length) {
   $unique_id = "";

   $i = 0;
   while ($i < $length) {
      $ch = chr(rand(48, 127));
      if (preg_match("/[a-zA-Z0-9]/", $ch)) {
         $unique_id .= $ch;
         $i++;
      }
   }
   return $unique_id;
}//getUniqueID
