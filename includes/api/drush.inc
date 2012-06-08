<?php

module_load_include('inc', 'api');
module_load_include('inc', 'update', 'update.manager');
include drupal_get_path('module', 'update') . '/update.authorize.inc';

/*
 * $modules is an array of tuples (module name, drupal version)
 */
 function patterns_drush_dlenable_modules_form($form, &$form_state, $modules, $modules_to_enable, $enable_dependencies) {  
  $form['#tree'] = TRUE;
  
  $form['all'] = array (
    '#type' => 'fieldset',
    '#title' => 'Download Options',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => TRUE, // enables tree datastrucutre
  );
 
  
  foreach($modules as $module) {
    // get release history
    $drupal_version = $module['drupal_version'];
    $module = $module['module'];
    
    $form['all'][$module] = array (
         '#type' => 'fieldset',
         '#title' => $module,
         '#collapsible' => TRUE,
         '#collapsed' => FALSE, // TODO set to TRUE after debugging
    );
    
    $xml = patterns_drush_get_release_history_xml($module, $drupal_version);
    
    if($xml == NULL) {
      $form['all'][$module]['error'] = array(
        '#markup' => 'An error occured while retrieving the release history. ' .
                     'Check the error messages for more information.',
      );
      continue;
    }
    // TODO: choose optimal solution
    
    // form for user

    // extract releases
    // Example for restricting:
    //  $releases = $xml->xpath("/project/releases/release[status='published']");
    $releases = $xml->xpath("/project/releases/release");
    $options = array('none');
    foreach($releases as $release) {
      // show information about release
      $opt =  $release->name;
      $opt .= ', Version: ' .  $release->version;
      $opt .= ', Version Extra: ' .  $release->version_extra;
      $opt .= ', Status: ' . $release->status;
      $opt .= ', ' . l('Release link',  $release->release_link);
      array_push($options, $opt);
      
      // show radios
      $form['all'][$module]['radios'] = array(
        '#type' => 'radios',
        '#title' => t('Choose the release you want to install' . $module),
        '#options' => $options,
      );
      
      // save requested drupal version
      $form['all'][$module]['drupal_version'] = array(
        '#type' => 'value',
        '#value' => $drupal_version,
      );
    }
  }
  
    //die(var_dump($modules_to_enable));
  if(!empty($modules_to_enable)) {
      $form['modules_to_enable'] = array (
        '#type' => 'fieldset',
        '#title' => 'Following modules are going to be enabled',
        '#collapsible' => FALSE,
        '#collapsed' => FALSE,
        '#tree' => TRUE, // enables tree datastrucutre
      );
    foreach($modules_to_enable as $module) {
      $form['modules_to_enable'][$module] = array(
        '#type' => 'value',
        '#value' => $module,
      );
    }

    $form['modules_to_enable']['list'] = array(
      '#markup' => implode('<br>', $modules_to_enable)
    );

    $form['modules_to_enable']['enable_dependencies'] = array(
      '#type' => 'value',
      '#value' => $enable_dependencies,
    );
  }
  
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => empty($modules_to_enable) ? t('Download') : t('Download & Enable'),
      '#submit' => array('patterns_drush_dlenable_modules_form_submit'),
  );

  return $form;
}

function patterns_drush_dlenable_modules_form_submit($form, &$form_state) {
  // array where all operations are saved for the batch
  $operations = array();
  $authorize = FALSE;
  
  //$output = array();
  if(!isset($form_state['values']['all']))
    $form_state['values']['all'] = array();
  foreach($form_state['values']['all'] as $module => $info) {
    echo $module;
    
    if($module == 'submit')
      continue;
      
    $drupal_version = $info['drupal_version'];
    $xml = patterns_drush_get_release_history_xml($module, $drupal_version);
    if($xml == NULL) {
      // should not happen, because it worked previously
      drupal_set_message(t('!project: Error while processing release history', array('!project' => $name)), 'error');
      continue;
    }
    $releases = $xml->xpath("/project/releases/release");
    // if none was chosen, do nothing
    $release = $form_state['values']['all'][$module]['radios'] - 1;
    if($release >= 0) {
      unset($context);
      $context['sandbox']['started'] = TRUE;
      // download and extract
      update_manager_batch_project_get($module, $releases[$release]->download_link, $context);
      $output[] = $context;
      $extract_directory = _update_manager_extract_directory();
      $project_real_location = $extract_directory . '/' . $module;
        
      // taken from update.manager.inc
      // update_manager_install_form_submit() 
      // check updater doc: http://api.drupal.org/api/drupal/includes!updater.inc/class/Updater/7
      //
      
      
      // Make sure the Updater registry is loaded.
      drupal_get_updaters(); // TODO: create own module updater
    
      // TODO: more informative error messages
      // TODO: error handling
      $project_location = $extract_directory . '/' . $module;
      try {
        $updater = Updater::factory($project_location);
      }
      catch (Exception $e) {
        drupal_set_message('Exception catched: ' . $e->getMessage(), 'error');
        return;
      }
    
      try {
        $project_title = Updater::getProjectTitle($project_location);
      }
      catch (Exception $e) {
        drupal_set_message('Exception catched: ' . $e->getMessage(), 'error');
        return;
      }
    
      if (!$project_title) {
        drupal_set_message(t('Unable to determine %project name.', array('%project' => $project)), 'error');
        return;
      }
    
      if ($updater->isInstalled()) {
        drupal_set_message(t('%project is already installed.', array('%project' => $project_title)));
        return;
      }
    
      $project_real_location = drupal_realpath($project_location);
      // If the owner of the directory we extracted is the same as the
      // owner of our configuration directory (e.g. sites/default) where we're
      // trying to install the code, there's no need to prompt for FTP/SSH
      // credentials. Instead, we instantiate a FileTransferLocal and invoke
      // update_authorize_run_install() directly.
      $arguments = array(
        'project' => $project_title,
        'updater_name' => get_class($updater),
        'local_url' => $project_real_location,
      );
      module_load_include('inc', 'update', 'update.authorize');
      $filetransfer = new FileTransferLocal(DRUPAL_ROOT);
        // call_user_func_array('update_authorize_run_install_modified', array_merge(array($filetransfer), $arguments));
      $operations[] = array(
        'update_authorize_batch_copy_project',
          array($project_title,
            get_class($updater),
            $project_real_location, // local url
            $filetransfer,
        ),
      );
      // check if we have enough rights - otherwise ask for authorization
      if (fileowner($project_real_location) == fileowner(conf_path()) && false ) {

      }
      // Otherwise, go through the regular workflow to prompt for FTP/SSH
      // credentials and invoke update_authorize_run_install() indirectly with
      // whatever FileTransfer object authorize.php creates for us.
      else {
        $authorize = TRUE;
        /*system_authorized_init('update_authorize_run_install', drupal_get_path('module', 'update') . '/update.authorize.inc', $arguments, t('Update manager'));
        $form_state['redirect'] = system_authorized_get_url();
        return;
        */
      }
    }
  }  
  
  // add operations to enable modules
  $modules_to_enable = array();
  $enable_dependencies = FALSE;
  if(isset($form_state['values']['modules_to_enable'])) {
    foreach($form_state['values']['modules_to_enable'] as $key => $value) {
      if($key != 'enable_dependencies')
        $modules_to_enable[] = $form_state['values']['modules_to_enable'][$key];
    }
    $enable_dependencies = $form_state['values']['modules_to_enable']['enable_dependencies'];
  }
  $operations[] = array(
    'patterns_drush_batch_module_enable',
    array($modules_to_enable, $enable_dependencies)
  );
    
  // die(var_dump($operations));
  // create batch with gathered operations and start it
  $batch = array(
    'title' => t('Installing modules'),
    'init_message' => t('Preparing to install'),
    'operations' => $operations,
    'finished' => 'update_authorize_install_batch_finished',
    'file' =>  drupal_get_path('module', 'Patterns') . '/includes/api/drush.inc',
    //'file' => drupal_get_path('module', 'update') . '/update.authorize.inc',
  );
  var_dump($batch);
 // die();


  if ($authorize) {
    system_authorized_init('patterns_drush_batch', drupal_get_path('module', 'Patterns') . '/includes/api/drush.inc', array($batch), t('Installing modules'));
    $form_state['redirect'] = system_authorized_get_url();
    return;
  }
  else {
   die('nay');
     batch_set($batch);
    // Invoke the batch via authorize.php.
    system_authorized_batch_process();
  }
}

function patterns_drush_batch($filetransfer, $batch) {
  var_dump($batch);
  foreach($batch['operations'] as &$op) {
    if($op[0] != 'patterns_drush_batch_module_enable')
      $op[1][3] = $filetransfer;
  }
  batch_set($batch);
  system_authorized_batch_process();
}

function patterns_drush_callback($filetransfer, $arguments_array) {
  foreach($arguments_array as $arguments) {
    update_authorize_run_install($filetransfer, $arguments);
  }
}
/**
 * module_enable wrapper for batch
 */
function patterns_drush_batch_module_enable($module_list, $enable_dependencies, &$context) {
  //echo 'module list <br>';
 // echo var_dump($module_list);
// die();
$context['finished'] = 1.0;
  //include('')
 // @module_enable($module_list);//, $enable_dependencies);
 // echo 'reaching';
 _update_batch_create_message($context['results']['log']['Return'], l('Click here to return.', 'admin/patterns/check/' . variable_get('patterns_pid', 0)));
  return;
}
 /**
 * Download the release history xml for the specified request.
 * based on drush's "_drush_pm_get_release_history_xml($request)" function
 * TODO: replace drush's error/warning messages with our own
 */
function patterns_drush_get_release_history_xml($name, $drupal_version) {
  $url = 'http://updates.drupal.org/release-history' . '/' . $name . '/' . $drupal_version;
  
  // Some hosts have allow_url_fopen disabled.
  // TODO: alternative solution?
  if (!$xml = @simplexml_load_file($url)) {
    drupal_set_message('Could not download release history.', 'error');
//  $filename = _drush_download_file($url);
//  $xml = simplexml_load_file($filename);
//  drush_op('unlink', $filename);
  }
  if (!$xml) {
    // We are not getting here since drupal.org always serves an XML response.
    drupal_set_message(t('!project: Could not download project status information from !url', array('!url' => $url, '!project' => $name)), 'error');
    return NULL;
  }
  if ($error = $xml->xpath('/error')) {
    // Don't set an error here since it stops processing during site-upgrade.
    //drush_log($error[0], 'warning'); // 'DRUSH_PM_COULD_NOT_LOAD_UPDATE_FILE',
    drupal_set_message(t('!project: Error returned as project status of information from !url', array('!url' => $url, '!project' => $name)), 'error');
    return NULL;
  }
  // Unpublished project?
  $project_status = $xml->xpath('/project/project_status');
  if ($project_status[0][0] == 'unpublished') {
    drupal_set_message(t("Project !project is unpublished and has no releases available.", array('!project' => $request['name'])), 'warning');
  }

  return $xml;
}

/*
 * Pick most appropriate release from XML list.
 *
 * @param array $request
 *   An array of version specifications as returned by pm_parse_project_version().
 * @param resource $xml
 *   A handle to the XML document.
 */

/*
function pm_parse_release($request, $xml) {
  $GET_DEV_VERSION = FALSE;
  
  // If that did not work, we will get the first published release for the
  // recommended major version.
  if ($recommended_major = $xml->xpath("/project/recommended_major")) {
    $xpath_releases = "/project/releases/release[status='published'][version_major=" . (string)$recommended_major[0] . "]";
    $releases = @$xml->xpath($xpath_releases);
  }
  
  // If there are recommended releases (no 'version_extra' elements), then use
  // only recommended releases.  Otherwise, use all; in this case, the
  // recommended release defaults to the latest published release with the
  // right recommended major version number.
  $recommended_releases = array();
  if (!empty($releases)) {
    foreach ($releases as $one_release) {
      if (!array_key_exists('version_extra', $one_release)) {
        $recommended_releases[] = $one_release;
      }
    }
  }
  if (!empty($recommended_releases)) {
    $releases = $recommended_releases;
  }
  $release_type = 'recommended';

  if ($GET_DEV_VERSION) {
    $releases = @$xml->xpath("/project/releases/release[status='published'][version_extra='dev']");
    $release_type = 'development';
  }
  if (drush_get_option('select', FALSE) || empty($releases)) {
    if (empty($releases)) {
      //drush_print(dt('There is no !type release for project !project.', array('!type' => $release_type, '!project' => $request['name'])));
    }
    $options = _drush_pm_download_releases_choice($xml, $request['name'], drush_get_option('all', FALSE), drush_get_option('dev', FALSE));
    $choice = drush_choice($options, dt('Choose one of the available releases:'));
    if ($choice) {
      $releases = $xml->xpath("/project/releases/release[status='published'][version='" . $choice . "']");
    }
    else {
      return FALSE;
    }
  }

  // First published release for the recommended major version is just the
  // first value in $releases.
  return (array)$releases[0];
}
*/
