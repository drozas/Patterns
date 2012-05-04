<?php
/**
 * @file
 * Export the components configuration to a Pattern file.
 *
 */

/**
 * TODO: finish Export documentation
 * TODO: code cleanup
 * TODO: Export through the Batch API
 */

/**
 * Use patterns_export_actions_from_form instead.
 * 
 * @deprecated
 * @see patterns_export_actions_from_form
 */
function patterns_api_extract_actions($form, $args = array(), $tag = NULL, $action = PATTERNS_MODIFY) {
  return patterns_export_actions_from_form($form, $args, $tag, $action);
}

/**
 * Extract the current state of a form.
 * Enter description here ...
 * @param unknown_type $form
 * @param unknown_type $args
 * @param unknown_type $action
 */
function patterns_export_actions_from_form($form, $args = array(), $tag = NULL, $action = PATTERNS_MODIFY) {

  set_error_handler("patterns_export_error_handler", E_ERROR | E_CORE_ERROR | E_COMPILE_ERROR | E_USER_ERROR);
  // Move this inside the Macro module
  // Macro should check that only one user is using that
  $form_state = array();
  $form_state['values'] = array();
  variable_set('patterns_extract_actions', TRUE);
  variable_set('macro_enabled', TRUE);
  variable_set('macro_submissions', array());
  $args = (!is_array($args)) ? array($args) : $args;

  $form_state['build_info']['args'] = $args;
  drupal_form_submit($form, $form_state);

  $result = macro_get_macro();

  variable_set('macro_enabled', FALSE);
  variable_set('patterns_extract_actions', FALSE);

  if (!empty($result) || !is_null($tag)) {
    $out = array();
    foreach ($result as $entry) {
      $a = $entry['values'];
      $out[] = array($action => array_merge(array('tag' => $tag), $a));
    }
    $result = $out;
  }

  return $result;
}

/**
 * Implements of hook_form_alter().
 * needed for deactivating actual submission of the forms when submitting, i.e.
 * we only want to know what gets submitted without the submission getting
 * processed
 */
function patterns_export_form_alter(&$form, $form_state, $form_id) {
  // Add the record callback on submit and delete all others
  if ($form_id != 'macro_import_macro' && variable_get('macro_enabled', FALSE) && variable_get('patterns_extract_actions', FALSE)) {
    $form['#submit'] = array();
    $form['#submit'][] = 'macro_record_macro';
  }
}

/**
 * In case error happens and script gets halted.
 */
function patterns_export_error_handler($errno, $errstr) {
  variable_set('macro_enabled', FALSE);
  variable_set('macro_submissions', array());
  variable_set('patterns_extract_actions', FALSE);
  restore_error_handler();
  // Normal error handler should take over from here.
  return FALSE;
}