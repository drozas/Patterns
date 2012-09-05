<?php

function patterns_export_php($filename, $tagmodules, $info, $to, $format) {
	
	list($sections, $modules) = patterns_export_core_php($tagmodules);

  // If some sections did not return any data
  // we remove them. We don't remove the modules!
  foreach ($sections as $s => $value) {
    if (empty($value)) {
      unset($sections[$s]);
    }
  }

  if (empty($sections)) {
    drupal_set_message(t('Export did not return any data.'), 'error');
    return FALSE;
  }
	    
 	return patterns_export_finalize_export($to,
 																	$sections,
 																	$info,
 																	$modules, 
 																	$format, 
 																	$filename);
}

/**
 * Constructs the abstract representation of the exported pattern
 * 
 * Returns the array of needed modules, and the actions of the 
 * pattern.
 * 
 * @param array $tagmodules An index of modules and tag to export
 * @return array $result An array containing modules and actions
 * 
 */
function patterns_export_core_php($tagmodules) {

  $result = array();
  $modules = array();
  foreach ($tagmodules as $module_name => $module) {
	  foreach ($module as $tag => $values) {
	   	if (!isset($values[PATTERNS_EXPORT]) || empty($values[PATTERNS_EXPORT])) {
	        continue;
	    }
			
	    if (isset($values[PATTERNS_FILES])) {
	      $files = (!is_array($values[PATTERNS_FILES])) ? array($values[PATTERNS_FILES]) : $values[PATTERNS_FILES];

	      foreach ($files as $file) {
	        require_once $file;
	      }
	    }
	
	    // TODO: the name of the component may be different from the
	    // the name of the module to enable. Cover this case
	    array_push($modules, $module_name);
	    
	    
	    if (!is_array($values[PATTERNS_EXPORT])) {
	      $sections[$tag] = call_user_func($values[PATTERNS_EXPORT]);
	    }
	    else {
	      foreach ($values[PATTERNS_EXPORT] as $f) {
	        $result[$tag] = isset($result[$tag]) ? $result[$tag] : array();
	        $sections[$tag] = array_merge($result[$tag], call_user_func($f));
	      }
	    }
	
	  }
  }

  $result[] = $sections;
  $result[] = $modules;

  return $result;
}

// TODO: implement the Batch export. Not working at the moment

/**
 * Constructs the abstract representation of the exported pattern
 * invoking the Batch API
 * 
 * Returns the array of needed modules, and the actions of the 
 * pattern.
 * 
 * @param array $tagmodules An index of modules and tag to export
 * @return array $result An array containing modules and actions
 * 
 */
function patterns_export_batch($filename, $tagmodules, $info, $to, $format) {

  $cur_sec = 1;
  $tot_sec = count($tagmodules);
  
	$result = array();
  $modules = array();
  $operations = array();
  foreach ($tagmodules as $module_name => $module) {
	  foreach ($module as $tag => $values) {
	   	if (!isset($values[PATTERNS_EXPORT]) || empty($values[PATTERNS_EXPORT])) {
	        continue;
	    }
			
	    $action_params = array($module_name, $tag, $values[PATTERNS_EXPORT]);
	    
	  	$action_params[] = isset($values[PATTERNS_FILES]) ? $values[PATTERNS_FILES]
	  																										 : array();
	    
			$operations[] = array('patterns_export_batch_core', $action_params);     
	  }


   	$progress_params = array(
                            '%section' => $module,
                            '%cur_sec' => $cur_sec,
                            '%tot_sec' => $tot_sec,
    );

    $progress = t('Running component "%section" (%cur_sec/%tot_sec), action @current out of @total', $progress_params);

    $batch = array(
    	'title' => t('Extracting configuration for %tag', array('%tag' => $tag)),
      'progress_message' => $progress,
      'finished' => 'patterns_export_batch_finish',
      'operations' => $operations,
      'init_message' => t('Initializing component "%section" (%cur_sec/%tot_sec)', array(
                           '%section' => $module,
                           '%cur_sec' => $cur_sec,
                           '%tot_sec' => $tot_sec,
                          )),
      );

    $_SESSION['patterns_export_batch_info'] = array('component' => $module);
    batch_set($batch);
    $cur_sec++;
  }
  
   

  return TRUE;
}

function patterns_export_batch_core($module_name, $tag, $export_funcs, $files = NULL, &$context) {
	
	$sections = array();
	
 	if (!empty($files)) {
		$files = (!is_array($files)) ? array($files) : $files;

	  foreach ($files as $file) {
	  	require_once $file;
	  }
	}
 
	if (!is_array($export_funcs)) {
		$sections[$tag] = call_user_func($values[PATTERNS_EXPORT]);
	}
	else {
		$result = array();
		foreach ($export_funcs as $f) {
			$result[$tag] = isset($result[$tag]) ? $result[$tag] : array();
	    $sections[$tag] = array_merge($result[$tag], call_user_func($f));
	  }
	}
	
	// TODO: the name of the component may be different from the
	// the name of the module to enable. Cover this case
	if (!isset($context['results']['modules'])) {
		$context['results']['modules'] = array();
	}
	if (!isset($context['results']['sections'])) {
		$context['results']['sections'] = array();
	}
		
	array_push($context['results']['modules'], $module_name);
	array_push($context['results']['sections'], $sections);
	
}

/**
 * Finishes a batch operation.
 * @TODO Doc.
 */
function patterns_export_batch_finish($success, $results, $operations) {
  $info = $_SESSION['patterns_export_batch_info'];
  $section = $results['module'];
  if (empty($results['abort'])) {
    foreach ($info as $key => $i) {
      drupal_set_message(t('Section "@section" of pattern "@pattern" ran successfully.', array('@pattern' => $i['title'],
                                                                                             '@section' => $section,
      )));
      $query_params = array(':time' => time(), // Note: time() != $_SERVER['REQUEST_TIME']
                            ':pid' => $key,
                            ':en' => PATTERNS_STATUS_ENABLED,
      );
      //db_query("UPDATE {patterns} SET status = :en, enabled = :time WHERE pid = :pid", $query_params);
    }
  }
  else {
    $pattern = reset($info);
    drupal_set_message(t('Section "@section" of pattern "@pattern" ran with errors.
      Check the error messages to get more details.',
      array('@pattern' => $pattern['title'], '@section' => $section,
            )),
     'error');
    drupal_set_message($results['error_message'], 'error');
  }

  unset($_SESSION['patterns_batch_info']);
  drupal_flush_all_caches();
}