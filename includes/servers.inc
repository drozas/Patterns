<?php
/**
 * @file
 * Functions related to importing and publishing patterns on the web.
 */

/**
 * Callback function for route /admin/patterns/publish/
 *
 * Sets the flag public to TRUE in the database and invokes the
 * hook_patterns_publish in all the modules implementing it.
 *
 * Redirects the user to the patterns home.
 *
 * @param mixed $pattern the id or the name of the pattern.
 */
function patterns_publish_pattern($pattern) {

  $pattern = patterns_utils_if_invalid_go_back($pattern);
  $result = patterns_db_publish_pattern($pattern);

  if ($result) {
    drupal_set_message(t('Pattern %pattern was marked as public.', array('%pattern' => @$pattern->name)));
  }
  else {
    drupal_set_message(t('An error occurred. Unable to mark pattern %pattern as public.', array('%pattern' => @$pattern->name)), 'error');
  }

  // Patterns API
  // Invoke all the modules who wants to do something with the published pattern
  module_invoke_all('patterns_publish', $pattern);

  drupal_goto('admin/patterns');
}

/**
 * Callback function for route /admin/patterns/unpublish/
 *
 * Sets the flag public to FALSE in the database and invokes the
 * hook_patterns_unpublish in all the modules implementing it.
 *
 * Redirects the user to the patterns home.
 *
 * @param mixed $pattern the id or the name of the pattern.
 */
function patterns_unpublish_pattern($pattern) {
  $pattern = patterns_utils_if_invalid_go_back($pattern);
  $result = patterns_db_unpublish_pattern($pattern);

  if ($result) {
    drupal_set_message(t('Pattern %pattern was marked as private.', array('%pattern' => @$pattern->name)));
  }
  else {
    drupal_set_message(t('An error occurred. Unable to mark pattern %pattern as private.', array('%pattern' => @$pattern->name)), 'error');
  }
  // Patterns API
  // Invoke all the modules who wants to do something with the unpublished pattern
  module_invoke_all('patterns_unpublish', $pattern);

  drupal_goto('admin/patterns');
}
