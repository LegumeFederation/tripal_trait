<?php
  $job_id = $_SESSION['trait_job_id'];
  
  $msg_file = "temporary://$job_id.log";
  if (file_exists($msg_file)) {
    $msgs = file_get_contents($msg_file);
    $html = nl2br($msgs);
  }
  else {
    $html = "<b>Please wait for results</b>";
  }
?>
<p>Tripal Trait upload results for job <?php echo $job_id?></p>
<p>
  <?php echo $html;?>
</p>
<p><a href="/admin/tripal/extension/tripal_trait/load_data">return</a></p>