<?php

/**
 * @file
 * Provides an application programming interface (API) for working the example
 * module
 */

/**
 * @defgroup tripal_example_api Example API
 * @ingroup tripal_api
 * @{
 * Provides an application programming interface (API) for working with features
 * @}
 */

// EXPLANATION: This file is used for housing any API function that your module
// creates. It is suggested you follow the naming style for Tripal API
// functions. Below are naming rules for Tripal API functions:
//
// 1) Prefix the function with 'chado_' for all insert, updates, delete or get
//    function on a table in the chado schema. Tripal should provide all of
//    these functions for tables that come with Chado. But, you can use this
//    prefix for API functions on custom tables that your module may add.
// 2) For all other functions prefix the function with your module name. So
//    for this example module the prefix would be 'tripal_example_'
// 3) A 'verb' should follow the function prefix indicating the action that
//    the function performs. Commonly used verbs are 'get', 'insert', 'update'
//    'associate', 'execute', etc. An exception is for functions that
//    test state rather than perform an action (e.g. does something exist).
//    For functions that test state use 'does' instead of a verb
//    (e.g. tripal_example_does_pub_exist)
// 4) Follow the verb with a short predicate describing what is being acted
//    upon.
//

function tripal_trait_run_data_loaders($job_id) {
  // Messages stored up here:
  $msgs = '';

$msgs .= "This log will contain all of the messages returned by the loading processes.\n\n";

  $pattern = "/tmp/$job_id.*";
  $files = glob($pattern);
  $msgs .= 'Found '. count($files) . " matching $pattern\n";
  foreach ($files as $file) {
    if (strstr($file, ".log")) {
      continue;
    }
    $msgs .= "Process file $file\n";
    $data = file($file);
    $sheet_type = getSheetType($data[0]);
    
    switch ($sheet_type) {
      case 'contact':
        $msgs .= tripal_trait_load_contact($data);
        break;
      case 'pub':
        $msgs .= tripal_trait_load_publication($data);
        break;
      case 'marker':
        $msgs .= tripal_trait_load_marker($data);
        break;
      case 'site-environment':
      case 'dataset':
      case 'stock':
      case 'mapset':
      case 'linkage_group':
      case 'QTL':
      case 'QTLtrait':
      case 'mapposition':
      case 'image':
      default:
        $msg = "The loader for the file of type $sheet_type is not yet implemented.";
        drupal_set_message($msg, 'warning');
        break;
    }//switch
  }//foreach file
  
  // Write messages to log file:
  $msg_file = "$job_id.log";
  $file = file_save_data($msgs, "temporary://$msg_file", FILE_EXISTS_REPLACE);
}//tripal_trait_run_data_loaders


function getSheetType($head) {
  $headers = explode("\t", $head);
  if ($headers[0] === 'publication_identifier') {
    return 'pub';
  }
  else if ($headers[0] === 'name_code') {
    return 'contact';
  }
  else if ($headers[0] === 'environment_identifier') {
    return 'site-environment';
  }
  else if ($headers[0] === 'dataset_identifier') {
    return 'dataset';
  }
  else if ($headers[0] === 'stock_identifier') {
    return 'stock';
  }
  else if ($headers[0] === 'map_identifier' 
            && $headers[1] === 'map_type') {
    return 'mapset';
  }
  else if ($headers[0] === 'map_identifier' 
            && $headers[1] === 'linkage_group') {
    return 'linkage_group';
  }
  else if ($headers[0] === 'marker_identifier') {
    return 'marker';
  }
  else if ($headers[0] === 'QTL_symbol') {
    return 'QTL';
  }
  else if ($headers[0] === 'trait_name') {
    return 'QTLtrait';
  }
  else if ($headers[0] === 'map_identifier' 
            && (strstr($headers[1], 'marker') || strstr($headers[1], 'QTL'))) {
    return 'mapposition';
  }
  else if ($headers[0] === 'image_URL') {
    return 'image';
  }
  else {
    return 'unknown';
  }
}//getSheetType