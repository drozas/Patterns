<?php
/**
 * @file
 * Drush Patterns module commands
 */

/**
 * Implements hook_drush_help().
 */
function patterns_drush_help($section) {
	switch ($section) {
		case 'drush:patterns-run':
			/** @todo Pattern files are searched for in the following locations: sites/[site name]/patterns, profiles/default/patterns, sites/all/patterns, sites/all/modules/contrib/patterns/patterns
			 --do we want to support this?*/
			return dt('Import and enable a pattern file.');
			break;
		case 'drush:patterns-enable':
			return dt('Enable a pattern by name or id');
			break;
		case 'drush:patterns-import':
			return dt('Import a pattern file.');
			break;
		case 'drush:patterns-list':
			return dt("List all available patterns.");
			break;
		case 'drush:patterns-info':
			return dt("Show all info on a given pattern.");
			break;
	}
}

/**
 * Implements hook_drush_command().
 */
function patterns_drush_command() {

	$dependencies = array('patterns', 'patterns_components', 'patterns_phpparser', 'patterns_yamlparser', 'patterns_xmlparser', 'token');
	$deps_export = $dependencies;
	array_push($deps_export, 'patterns_export');
	
	
	$items['patterns-quickrun'] = array(
    'callback' => 'drush_patterns_quickrun',
    'description' => 'Load a pattern file and executes it without saving it into the database',
    'examples' => array(
      'drush patterns-quickrun file_name [format] [options] [mode]' => 'Load a pattern file and executes it without saving it into the database',
		),
    'arguments' => array(
      'file_name' => 'The path to the a pattern file',
    	'format' => 'Optional. The format of the pattern. If not specified, it will be determined by the extension.',
      'params' => 'Optional. Additional options for the pattern execution.',
    	'mode' => 'Optional. Execute via batch or php',        
		),
    'drupal dependencies' => $dependencies,
    'core' => array('7'),
	);

	$items['patterns-enable'] = array(
    'callback' => 'drush_patterns_enable',
    'description' => 'Enable the specified pattern name',
    'examples' => array(
      'drush patterns-enable pattern_name [first-update|always|update|first|never]' => 'Enable the pattern specified by pattern id or pattern name'
      ),
    'arguments' => array(
      'id' => 'A pattern id or name',
      ),
    'drupal dependencies' => $dependencies,
    'core' => array('7'),
  );

  $items['patterns-export'] = array(
    'description' => 'Import the specified pattern file.',
    'examples' => array(
      'drush patterns-export path/to/patterns/file.[xml/yaml]' =>
        'Import the specified pattern file.',
      	'drush patterns-import path/to/pattern/file.[xml/yaml] pattern_name' => 'Import the specified pattern file and give it a machine readable name'
     ),
    'drupal dependencies' => $deps_export,
    'core' => array('7'),
   );
   
  $items['patterns-import'] = array(
    'callback' => 'drush_patterns_import_file',
    'description' => 'Import the specified pattern file.',
    'examples' => array(
      'drush patterns-import path/to/patterns/file.[xml/yaml]' =>
        'Import the specified pattern file.',
      	'drush patterns-import path/to/pattern/file.[xml/yaml] pattern_name' => 'Import the specified pattern file and give it a machine readable name'
      ),
    'arguments' => array(
      'file' => 'The path to a pattern file',
      'format' => 'A valid pattern format, e.g. xml or yaml.'
    ),
    'drupal dependencies' => $dependencies,
    'core' => array('7'),
   );
   
   $items['patterns-import-remote'] = array(
    'callback' => 'drush_patterns_import_file_remote',
    'description' => 'Import a remote pattern file.',
    'examples' => array(
      'drush patterns-import-remote http://myserver.org/mypattern.yaml' =>
        'Import a remote pattern file.',
      'drush patterns-import-remote http://myserver.org/mypattern [format]' =>
        'Import a remote pattern file and specify the format.',
    ),
    'arguments' => array(
      'uri' => 'The uri to a pattern file',
      'format' => 'A valid pattern format, e.g. xml or yaml.'
    ),
    'drupal dependencies' => $dependencies,
    'core' => array('7'),
   );

   $items['patterns-run'] = array(
    'callback' => 'drush_patterns_run',
    'description' => 'Import and enable the specified pattern file',
    'examples' => array(
      'drush patterns-run path/to/patterns/file.xml' =>
        'Import and enable the specified pattern file',
      'drush patterns-run path/to/pattern/file.xml pattern_name' => 'Import and enable the specified pattern file and give it a machine readable name'
      ),
    'arguments' => array(
      'pattern_file' => 'The path to a pattern file',
      'pattern_name' => 'An optional machine-readable pattern name to override the one automatically generated from the title field of the pattern'
      ),
    'drupal dependencies' => $dependencies,
    'core' => array('7'),
      );

   $items['patterns-list'] = array(
    'callback' => 'drush_patterns_list',
    'description' => "List all available patterns."
   );

   $items['patterns-info'] = array(
    'callback' => 'drush_patterns_info',
    'description' => "Show all info on a given pattern.",
    'arguments' => array(
      'id' => "The pattern id or name of the pattern for which to show info.",
    )
   );

   $items['patterns-form-helper'] = array(
    'description' => 'Enable/disable patterns form helper',
    'arguments' => array(
      'action' => "Valid values: enable, disable.",
    )
   );

   $items['patterns-allow-publish'] = array(
    'description' => 'Enable/disable patterns publishing',
    'arguments' => array(
      'action' => "Valid values: enable, disable.",
    )
   );

   $items['patterns-paths'] = array(
    'description' => "List all paths where patterns will be looked for",
    'drupal dependencies' => array('patterns'),
   );
   
   $items['patterns-components'] = array(
    'description' => "List all the available patterns components",
    'drupal dependencies' => array('patterns'),
   );

   $items['patterns-publish'] = array(
    'description' => "Publish a pattern.",
    'arguments' => array(
      'pid' => "The ID of the pattern to publish.",
    ),
    'drupal dependencies' => array('patterns'),
   );

   $items['patterns-unpublish'] = array(
    'description' => "Unpublish a pattern.",
    'arguments' => array(
      'pid' => "The ID of the pattern to unpublish.",
    ),
    'drupal dependencies' => array('patterns'),
   );
   
	return $items;
}

// Callbacks


function drush_patterns_export() {
	drush_print(dt('Patterns export.'));
	
	if (!patterns_utils_is_module_enabled('patterns_export')) {
		drush_set_error(dt('Please enable module patterns_export first'));
		return FALSE;
	}
  
	$args = func_get_args();
	if (empty($args)) {
		return drush_set_error(dt('You must specify the export mode and the output file'));
	}
		
  $mode = array_shift($args);
	if (!patterns_export_is_valid_mode($mode)) {
		$modes_string = '[' . PATTERNS_EXPORT_TO_DB;
		$modes_string.= '|' . PATTERNS_EXPORT_TO_FILE;
		$modes_string.= '|' . PATTERNS_EXPORT_TO_ZIP . ']';
		return drush_set_error(dt('You must specify a valid export mode: ' . $modes_string));
	}
	
	// out file
	$out = array_shift($args);
	$format = PATTERNS_FORMAT_UNKNOWN;
	
	if ($mode === PATTERNS_EXPORT_TO_ZIP) {
		$format = patterns_io_get_format_from_file(NULL, array_shift($args));
		if (!$format) {
			return drush_set_error(dt('You must specify the format when exporting to Zip archive.'));
		}
	}
	
	$format = patterns_io_get_format_from_file($out, $format);
	if (!$format) {
		return FALSE;
	}

  if (empty($args)) {
    drush_print(dt('Exporting all components'));
    $exports = patterns_moduletags_get_index(NULL, TRUE, TRUE);
	}
	else {
		$exports = patterns_drush_filter_components($args);
	}
	
	$result = patterns_export_start_engine($out, $exports, $info, $mode, $format, 'php');
  
	if (!$result) {
		drush_set_error(dt('An error occurred while exporting. Export may have failed.'));
	}
	else {
		drush_print(dt('Patterns exported successfully'));
	}
  
	//drush_print(print_r($export_functions, true));

}

/**
 * Run a pattern file without importing it into the database
 *
 * @param string $file The path to the pattern file
 * @param string $format Optional. The format of the pattern.
 * @param string $mode Optional. batch or php. Defaults php.
 * @param unknown_type $params
 */
function drush_patterns_quickrun($file, $format = PATTERNS_FORMAT_UNKNOWN, $mode = 'php', $params = array()) {
	drush_print(dt('Patterns quick-run.'));
	$format = patterns_drush_get_file_format($file, $format);
	if (!$format) {
		return drush_set_error('Invalid pattern format. Aborting');
	}
	drush_print(dt('Format: ' . $format));

	return patterns_load_file_and_start_engine($file, $format, $mode, $params);
}

/**
 * Imports, Enables, and Runs the specified pattern file
 *
 * @param string pattern file pathname
 * @param string optional machine readable pattern name
 */
function drush_patterns_run($name, $format = PATTERNS_FORMAT_UNKNOWN, $mode = 'php', $params = array()) {
	drush_print(dt('Running pattern: ' . $name));
	
	$pattern = patterns_get_pattern($name);
	if (!$pattern) {
		if (!drush_file_not_empty($name)) {
			return drush_set_error(dt('Pattern was not found in the database nor in the file system.'));
		}
		
		if (!drush_confirm(dt("Pattern '$name' has not been imported. Do you want to import it now? "))) {
			return drush_print(dt('patterns-run has been canceled.'));
		}
			
		$format = patterns_drush_get_file_format($file, $format);
		if (!$format) {
			return drush_set_error('Invalid pattern format. Aborting');
		}
		$result = patterns_io_import_file($name, $format);
		if (!$result) {
			return drush_set_error(dt('Importing pattern failed'));
		}
		
		$name = patterns_io_get_name_from_file($name);
		// Check if the file has a valid extension
  	// and in case add the format at the end
  	if (!_patterns_io_file_has_valid_extension($name)) {
    	$name = $name . '.' . $format;
  	}
		
		$pattern = patterns_get_pattern($name); 
	}
	
	return drush_patterns_run_pattern($pattern, $mode, $params);
}

/**
 * Enables the specified pattern
 * @param string $id pattern name or pattern id
 * @param string $subpattern_mode Choice of first-update, always, update,
 * first, never. Default is first-update.
 *
 * @todo Currently the function will barge through and run all sub-patterns by
 * default. will want to revisit this and perhaps implement a second parameter
 * to handle whether to do so or not.
 */
function drush_patterns_enable($id, $subpattern_mode = 'first-update') {
	drush_print(dt("Enabling pattern '$id'"));

	$pattern = patterns_get_pattern($id);

	if (!$pattern) {
		return drush_set_error(dt("Cannot enable pattern '$id' as it could not be found in the list of registered patterns. Did you import it first?"));
	}

	/** defaulting to batch mode & always running subpatterns
	 * run-subpatterns choices are:
	 'first-update' => t('only if disabled or if updated since last run (recommended)'),
	 'always' => t('always'),
	 'update' => t('only if updated since last run'),
	 'first' => t('only if disabled'),
	 'never' => t("don't run sub-patterns at all"),
	 @todo make these constants define() statements in patterns.module
	 */
	$result = patterns_execute_pattern_drush($pattern, array('run-subpatterns' => $subpattern_mode, 'pid' => $pattern->pid, 'confirm' => 1, 'op' => 'Confirm', 'submit' => 'Confirm'));

	if (!$result) {
		return drush_set_error(dt("Enabling pattern '$pattern->pid:$pattern->name' failed"));
	}
	drush_print(dt("Success! Enabled pattern '$pattern->pid:$pattern->name'"));
	return TRUE;
}


/**
 * patterns list command callback.
 */
function drush_patterns_list() {
	$patterns = patterns_io_get_patterns();
	$patterns = $patterns[PATTERNS_STATUS_OK];
	$pipe = array();
	$rows[] = array(dt('Id'), dt('Name'), dt('Title'), dt('Status'), dt('Version'));
	foreach ($patterns as $pid => $pattern) {
		$rows[] = array($pattern->pid, $pattern->name, $pattern->title, $pattern->status, $pattern->info['version']);
		$pipe[] = "$pattern->title";
	}

	drush_print_table($rows, TRUE);
	drush_print_pipe(implode(' ', $pipe));
}

/**
 * patterns list command callback.
 * @param mixed int|string $id The pattern id or name of the pattern
 */
function drush_patterns_info($id) {
	$pattern = patterns_get_pattern($id);
	if (!$pattern) {
		return drush_set_error(dt("There is no pattern registered with id or name '$id'"));
	}
	$info = "";
	$info .= sprintf("  %-18s: %s\n", 'PID', $pattern->pid);
	$info .= sprintf("  %-18s: %s\n", 'Name', $pattern->name);
	$info .= sprintf("  %-18s: %s\n", 'Description', $pattern->description);
	$info .= sprintf("  %-18s: %s\n", 'File', $pattern->file);
	$info .= sprintf("  %-18s: %s\n", 'Status', $pattern->status);
	$info .= sprintf("  %-18s: %s\n", 'Public', $pattern->public);
	$info .= sprintf("  %-18s: %s\n", 'Updated', $pattern->updated);
	$info .= sprintf("  %-18s: %s\n", 'Enabled', $pattern->enabled);

	if (is_array($pattern->pattern['info'])) {
		foreach ($pattern->pattern['info'] as $key => $value) {
			$info .= sprintf("  %-18s: %s\n", ucfirst($key), $value);
		}
	}
	if (is_array($pattern->pattern['modules'])) {
		$info .= sprintf("  %-18s: %s\n", 'Modules', implode(', ', $pattern->pattern['modules']));
	}

	drush_print($info);
}

/**
 * patterns list command callback.
 */
function drush_patterns_form_helper($action) {
	if ($action == 'enable') {
		variable_set('patterns_form_helper', TRUE);
	}
	else {
		variable_set('patterns_form_helper', FALSE);
	}
}

/**
 * patterns list command callback.
 */
function drush_patterns_allow_publish($action) {
	if ($action == 'enable') {
		drush_print("Patterns publishing patterns enabled.");
		variable_set('patterns_allow_publish', TRUE);
	}
	else if ($action == 'disable' ) {
		drush_print("Patterns publishing patterns disabled.");
		variable_set('patterns_allow_publish', FALSE);
	}
	else {
		drush_set_error(dt("Unrecognized option."));
	}
}

/**
 * List of patterns path command callback.
 */
function drush_patterns_paths() {
	drush_print("List of path containing Patterns files.");
	$paths = patterns_path_get_patterns_dirs();
	foreach ($paths as $path) {
		drush_print("- $path", 4);
	}

	drush_print_pipe(implode(', ', $paths));
}

/**
 * List of patterns path command callback.
 */
function drush_patterns_components() {
	drush_print("List of available Patterns components.");
	$paths = patterns_io_list_components_names(TRUE);
	foreach ($paths as $path) {
		drush_print("- $path", 4);
	}

	drush_print_pipe(implode(', ', $paths));
}

/**
 * patterns publish command callback.
 */
function drush_patterns_publish($pid) {
	drush_print("Publishing pattern.");
	$result = patterns_db_publish_pattern($pid);
	if ($result) {
		drush_print("Operation was successful");
	}
	else {
		drush_set_error(dt("An error occurred, patterns was not marked as public."));
	}
}

/**
 * patterns unpublish command callback.
 */
function drush_patterns_unpublish($pid) {
	drush_print("Unpublishing a pattern.");
	$result = patterns_db_unpublish_pattern($pid);
	if ($result) {
		drush_print("Operation was successful");
	}
	else {
		drush_set_error(dt("An error occurred, patterns public status could not be changed."));
	}
}

function drush_patterns_run_pattern($pattern, $mode = 'php', $params = array()) {
	$result = patterns_start_engine($pattern, $params, $mode);
	if ($result) {
		if ($mode === 'batch') {
			$batch = &batch_get();
			$batch['progressive'] = FALSE;
			drush_backend_batch_process();
		}
		drush_print(dt('Pattern execution started.'));
	}
	else {
		drush_set_error(dt('An error occurred while processing the pattern file.'));
	}
	return $result;
}

/**
 * Imports the specified patterns file
 * @param string path to the patterns file
 * @param string optional machine readable name for the pattern you are importing
 * @return mixed bool|string FALSE upon failure; the name of the imported
 * pattern upon success
 */
function drush_patterns_import_file($file, $format = PATTERNS_FORMAT_UNKNOWN) {
	drush_print(dt("Importing local pattern file: " . $file));
	return patterns_io_import_file($file, $format);
}

/**
 * Imports the specified patterns file
 * @param string path to the patterns file
 * @param string optional machine readable name for the pattern you are importing
 * @return mixed bool|string FALSE upon failure; the name of the imported
 * pattern upon success
 */
function drush_patterns_import_file_remote($file, $format = PATTERNS_FORMAT_UNKNOWN) {
	drush_print(dt("Importing pattern file from remote uri: " . $file));
	return patterns_io_import_file_remote($file, $format);
}


// HELPER FUNCTIONS

function patterns_drush_get_file_format($file, $format = PATTERNS_FORMAT_UNKNOWN) {
	$format = patterns_io_get_format_from_file($file, $format, FALSE);
	if (!$format) {
		$format = drush_prompt(dt("Unable to recognize the pattern format. Please specify it")); 
		if (!patterns_parser_exists($format)) {
			return FALSE;
		}
	}
	return $format;
}

function patterns_load_file_and_start_engine($file, $format = PATTERNS_FORMAT_UNKNOWN, $mode, $params) {
	$pattern = patterns_io_load_pattern_from_file($file, $format);
	$params = array('run-subpatterns' => TRUE);
	drush_patterns_run_pattern($pattern, $mode, $params);
}

/**
 * Validate, and filters user input for export components
 * 
 * The input is of the type
 * 
 * 		component:export_func_id
 * 
 * and returns an array of component => export_func_name
 * 
 * @param array $out
 */
function patterns_drush_filter_components($components) {
	$out = array();
	// Find the components.
  $all_components = patterns_io_list_components_names(TRUE);
  //drush_print(print_r($all_components, true));
      
    foreach ($components as $c) {
    	$func = NULL;
    	$func_name = NULL;
    	$t = NULL;
    	$tag = NULL;
    	$tag_name = NULL;
    	$export_func = NULL;
    	
      list($component, $tag, $export_func) = explode('.', $c);
      
      if (!in_array($component, $all_components)) {
      	drush_set_error(dt('Component not found: ' . $component));
      	continue;
      }
      
      $info = call_user_func($component . '_patterns');
      
      if (empty($info)) {
      	drush_set_error(dt('An error occurred while retrieving information from ' . $component));
      	continue;
      }
      
      // If no tag was specified, get all export functions
      if (empty($tag)) {
      	foreach($info as $tag => $t) {
      		if (!isset($t[PATTERNS_EXPORT]) || empty($t[PATTERNS_EXPORT])) {
      		 	continue;
      		}
      		foreach ($t[PATTERNS_EXPORT] as $func_name => $func) {

      			$out[$component][$tag][PATTERNS_EXPORT][$func_name] = $func; 
      		}
      		
      		// add files
      		if (isset($t[PATTERNS_FILES])) {
      			$out[$component][$tag][PATTERNS_FILES] = $t[PATTERNS_FILES]; 
      		}
      	}		
      	continue;
      }
      
      if (!isset($info[$tag])) {
      	drush_set_error(dt('Component "' . $component . '" has no tag "' . $tag . '" defined.'));
      	continue;
      }
      
      if (!isset($info[$tag][PATTERNS_EXPORT]) || empty($info[$tag][PATTERNS_EXPORT])) {
      	drush_set_error(dt('Component "' . $component . '", tag "' . $tag . '" has no export function defined.'));
      	continue;
      }
      
    	// If no export function was specified, get all export functions for the tag 
      if (empty($export_func)) {
      	foreach ($info[$tag][PATTERNS_EXPORT] as $func_name => $func) {
      		$out[$component][$tag][PATTERNS_EXPORT][$func_name] = $func;
      	}	
      	// add files
      	if (isset($info[$tag][PATTERNS_FILES])) {
        	$out[$component][$tag][PATTERNS_FILES] = $info[$tag][PATTERNS_FILES]; 
        } 
      	
      	continue;
      }
      
      if (!isset($info[$tag][PATTERNS_EXPORT][$export_func])) {
      	drush_set_error(dt('Component "' . $component . '", tag "' . $tag . '" has no xport function "' . $export_func . '"'));
      	continue;      	
      }
      
  		$out[$component][$tag][PATTERNS_EXPORT][$export_func] = $info[$tag][PATTERNS_EXPORT][$export_func];
  		
  		// add files
  		if (isset($info[$tag][PATTERNS_FILES])) {
      	$out[$component][$tag][PATTERNS_FILES] = $info[$tag][PATTERNS_FILES]; 
      } 
    }
    
	return $out;

}
