<?php
/**
 * @file
 * Patterns modules page theme functions.
 */

/**
 * Lists the modules used by a particular pattern.
 *
 * @TODO Doc.
 */
function patterns_modules_page($pid) {
  $form_state['build_info']['args'] = array($pid);
  return drupal_render(drupal_build_form('patterns_modules_page_form', $form_state));
}


function patterns_modules_page_form($form, &$form_state, $pid) {
  // check if this is the first time or if we are rebuilding the form
  if(!isset($form_state['values']['step'])) {
    $form_state['values']['step'] = 1;
  }
  else {
    // check what button was clicked and which operation to execute
    // download button?
    if($form_state['values']['submit_dl'] == $form_state['triggering_element']['#value']) {
      // create array of tuples: (module, drupal_version)
      $modules = array();
      foreach($form_state['values']['modules'] as $module => $info) {
        if($info['download'] == 1) {
          $modules[] = array('module' => $module, 'drupal_version' => $info['drupal_version']);
        }
      }
      $form_state2;
      $form = patterns_drush_download_modules_form(NULL, $form_state2, $modules);
          
      return $form;
    }
    // enable button?
    else if($form_state['values']['submit_en'] == $form_state['triggering_element']['#value']) {
      drupal_set_message('Not yet implemented', 'error');
    }
    // download and enable button?
    else if($form_state['values']['submit_dlen'] == $form_state['triggering_element']['#value']) {
      drupal_set_message('Not yet implemented', 'error');
    }
  }
  
  // needed for formatting
  $form['available'] = array(
      '#markup' => t(''),
  );
  
  $pattern = patterns_utils_if_invalid_go_back($pid);

  drupal_set_title($pattern->title . ' ' . t('(Pattern Modules)'));
  $modules = isset($pattern->pattern['modules']) ? $pattern->pattern['modules'] : array();
  $drupal_version = isset($pattern->pattern['info']['core']) ? $pattern->pattern['info']['core'] : 'core undefined';
  $modules = is_array($modules) ? $modules : array($modules);
  $modules_info = system_rebuild_module_data();
  $modules_list = module_list();

  $not_available = FALSE;
  
  $form['modules'] = array();
  $form['#tree'] = TRUE;
  $form['step'] = array(
    '#type' => 'value',
    '#value' => $form_state['values']['step'],
  );
  
  // Get module name, whether its to be disabled or enabled,
  // whether the module is available or not, and whether it is
  // currently enabled or not
  foreach ($modules as $module) {
    $module = is_array($module) ? $module['value'] : $module;
    $available = array_key_exists($module, $modules_info);
    $enabled = array_key_exists($module, $modules_list);
    $drupal_version = isset($pattern->pattern['info']['core']) ? $pattern->pattern['info']['core'] : 'core undefined';

    $form['modules'][$module]['m'] = array(
      '#markup' => t($module),
    );
      
    $form['modules'][$module]['module'] = array(
      '#type' => 'value',
      '#value' => $module,
    );
    
    $form['modules'][$module]['drupal_version'] = array(
      '#type' => 'value',
      '#value' => $drupal_version,
    );
    
    if($available) {
      $form['modules'][$module]['available'] = array(
        '#markup' => t(' available'),
      );
    }
    else {
      $form['modules'][$module]['available'] = array(
        '#markup' => t(' not available'),
      );
    }
    
    // enable checkbox
    $form['modules'][$module]['enable'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable?'),
      '#disabled' => !$available || $enabled,
    );
    
    // download checkbox
    $form['modules'][$module]['download'] = array(
      '#type' => 'checkbox',
      '#title' => t('Download?'),
      '#disabled' => $available,
    );
    
    if(!$available) {
      $not_available = TRUE;
    }
  }
  
  if ($not_available) {
    //drupal_set_message(t('Some modules are not available, please download them before running this pattern.'), 'error');
    $form['available'] = array(
      '#markup' => t('Some modules are not available, please download them before running this pattern.<br>'),
    );
  }
  else {
    //drupal_set_message(t('All modules required by this module are available. Click !here to run this pattern.', array('!here' => l(t('here'), 'admin/patterns/enable/' . $pid))));
    $form['available'] = array(
      '#markup' => t('All modules required by this module are available. Click !here to run this pattern.<br>', array('!here' => l(t('here'), 'admin/patterns/enable/' . $pid))),
    );
    return $form;
  }
  
  // buttons
  $form['submit_dl'] = array(
    '#type' => 'submit',
    '#value' => t('Download'),
    '#submit' => array('patterns_modules_page_submit'),
  );
  
  $form['submit_en'] = array(
    '#type' => 'submit',
    '#value' => t('Enable'),
    '#submit' => array('patterns_modules_page_submit'),
  );
  
  $form['submit_dlen'] = array(
    '#type' => 'submit',
    '#value' => t('Download & Enable'),
    '#submit' => array('patterns_modules_page_submit'),
  );
  
  return $form;
}

// needed for redirecting to another form
function patterns_modules_page_submit($form, &$form_state) {
  $form_state['values']['step'] = $form_state['values']['step'] + 1;
  $form_state['rebuild'] = TRUE;  
}