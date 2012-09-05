<?php


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
function patterns_export_php($tagmodules) {
	//drush_print(print_r($tagmodules, true));

  $result = array();
  $modules = array();
  foreach ($tagmodules as $module_name => $module) {
	  foreach ($module as $tag => $values) {
	   	if (!isset($values[PATTERNS_EXPORT])) {
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
function patterns_export_batch($tagmodules) {

  $cur_sec = 1;
  $tot_sec = count($tagmodules);
  foreach ($tagmodules as $tag => $values) {
    if (!isset($values[PATTERNS_EXPORT])) {
        continue;
    }
    if (isset($values[PATTERNS_FILES])) {
      $files = (!is_array($values[PATTERNS_FILES])) ? array($values[PATTERNS_FILES]) : $values[PATTERNS_FILES];
      foreach ($files as $file) {
        require_once $file;
      }
    }
    $module = $values['module'];
    $func = array($values[PATTERNS_EXPORT][PATTERNS_EXPORT_ALL]);
    //foreach ($tags as $tag => $forms) {



      $progress_params = array(
                            '%section' => $module,
                            '%cur_sec' => $cur_sec,
                            '%tot_sec' => $tot_sec,
      );

      $progress = t('Running component "%section" (%cur_sec/%tot_sec), action @current out of @total', $progress_params);

      $batch = array(
        //'title' => t('Processing section %section of pattern %pattern', array('%section' => $section, '%pattern' => $info['title'])),
        'title' => t('Extracting configuration for %tag', array('%tag' => $tag)),
        'progress_message' => $progress,
        'finished' => 'patterns_export_batch_finish',
        'operations' => array($func),
        'init_message' => t('Initializing component "%section" (%cur_sec/%tot_sec)', array(
                            '%section' => $module,
                            '%cur_sec' => $cur_sec,
                            '%tot_sec' => $tot_sec,
                          )),
      );

      if (isset($values[PATTERNS_FILES]) && !empty($values[PATTERNS_FILES])) {
        //$batch['file'] = current($values[PATTERNS_FILES]); // should be an array or not?
        $batch['file'] = drupal_get_path('module', 'patterns_components') . 'components/taxonomy.inc';
      }



    //}

    $_SESSION['patterns_export_batch_info'] = array('component' => $module, 'tag' => $tag);
    batch_set($batch);
    $cur_sec++;
  }

  return TRUE;
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
      db_query("UPDATE {patterns} SET status = :en, enabled = :time WHERE pid = :pid", $query_params);
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