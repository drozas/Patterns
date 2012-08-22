<?php
/**
 * @file
 * Importing Patterns from a file or using an URL.
 */

/**
 * Helper function to be called in one of the import validation handlers.
 */
function _patterns_io_import_check_dirs($dir = NULL) {
  // Check Patterns Directory.
  if (!_patterns_io_is_patterns_dir_ready($dir, FILE_CREATE_DIRECTORY)) {
    $message = t("Destination folder does not exist or is not writable: %path<br />", array('%path' => $dir)); // TODO: move this out for performance
    $link = l(t('status report'), 'admin/reports/status');
    $message .= t('You might want to check the !link to ensure your files directory exists and is writable.', array('!link' => $link));
    form_set_error('pattern_file', $message);
    return FALSE;
  }
}

/**
 * Helper function to be called in one of the import validation handlers.
 */
function _patterns_io_import_check_name($name = '') {
  // Name Validation.
  $errors_on_name = _patterns_parser_analyze_pattern_name($name);
  if (!empty($errors_on_name)) {
    form_set_error('name', t('Errors in the name of the pattern: ' . implode(', ', $errors_on_name))); // TODO: use proper t()
    return FALSE;
  }

  if ($obj = patterns_db_load_pattern_from_name($name)) {
    form_set_error('name', t('A pattern file with the same name already exists: !name', array('!name' => $name)));
    return FALSE;
  }
}

/**
 * Gets the pattern format from a file, and checks that it actually exists
 * 
 * @param unknown_type $file
 * @param unknown_type $format
 */
function patterns_io_get_format_from_file($file, $format = PATTERNS_FORMAT_UNKNOWN) {
	if (!file_exists($file) && is_readable($file)) {
		drupal_set_message(t('File does not exists or is not readable. Aborting.'), 'error');
		return FALSE;
	}
	if ($format === PATTERNS_FORMAT_UNKNOWN) {
		$format = pathinfo($file, PATHINFO_EXTENSION);
	}
	if (!patterns_parser_exists($format)) {
		drupal_set_message(t('Unknown pattern format: %format. Please check available parsers.', array('%format' => $format)));
		return FALSE;
	}
	return $format;
}

function patterns_io_import_file($file, $format = PATTERNS_FORMAT_UNKNOWN) {

  if (!patterns_parser_ready()) {
    $messages = t('No available patterns parser was found.');
    $messages .= t(' Go to the !modules page to enable more Patterns parsers.', array('!modules' => l(t('modules'), 'admin/modules')));
    drupal_set_message($messages,'warning');
    return FALSE;
  }

  $destination = patterns_path_get_files_dir();

  // If not automatically returns
  _patterns_import_check_dirs($destination);
  _patterns_import_check_name($form_state['values']['name']);

  // Retreiving the format
  
	$format = patterns_io_get_format_from_file($file, $format);
	
	if (!$format) {
		return FALSE;
	}
  
  // FILE or URI

    // Local
    if (isset($form_state['values']['local'])) {
      $validators = array('file_validate_extensions' => array(implode(' ', patterns_parser_get_formats(FALSE))));
      if ($file = file_save_upload('pattern_uri', $validators)) {
        $form_state['values']['content'] = file_get_contents($file->uri);
        $form_state['pattern_uri'] = $file;
        $form_state['pattern_uri']->destination = $destination;
      }
      else {
        $err_file = TRUE;
      }
    }
    
    // Remote File
    else {

      if (!ini_get('allow_url_fopen')) {
        form_set_error('url', t('allow_url_fopen must be enabled in your php configuration in order to use this feature.'));
        return FALSE;
      }

      if ($content = file_get_contents($form_state['values']['pattern_uri'])) {
        $form_state['values']['content'] = $content;
      }
      else {
        $err_file = TRUE;
      }
    }

    if ($err_file) {
      form_set_error('pattern_uri', t('Failed to retrieve the pattern specified: %uri. Check your file and try again.', array('%uri' => $form_state['values']['pattern_uri'])));
      return FALSE;
    }
  }
  // End FILE or URI



  // Parse
  $pattern = patterns_parser_parse($form_state['values']['content'], $format);

  if (!$pattern) {
    form_set_error('content', t('Pattern could not parsed. Please make sure it is well formed.'));
    return FALSE;
  }


  $patternscan = patterns_scan_pattern($pattern);
  if (!_patterns_scan_validate_patternscan($patternscan)) {
    $errors_on_scan = _patterns_scan_analyze_patternscan($patternscan);
    form_set_error('name', t('Error parsing the YAML file: ' . implode(', ', $errors_on_scan))); // TODO: proper t()
  }


    $success = patterns_io_save_pattern($pattern,
                                        $name,
                                        $format,
                                        $form_state['values']['destination'],
                                        $form_state['values']['content']
    );
  }

  if ($success) {
    $link = l('here', 'admin/patterns/edit/' . $name); // TODO: t()
    $msg = t('Pattern %name successfully imported. ', array('%name' => $name)); // TODO: proper t()
    $msg.= t('Click ') . $link . t(' to view it.'); // TODO: t()
    drupal_set_message($msg);
  }
  else {
    drupal_set_message(t('File %name could not be saved on the server. Import failed.', array('%name' => $name)), 'error');
  }
}