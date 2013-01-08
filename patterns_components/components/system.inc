<?php
/*
 * TODO: Only set a theme default if 'default' is 1.
 * TODO: Check if it is possible to simplify the prepare hook. Check variables delete validation case
 * @file
 * Patterns component for system related operations.
 */

/**
 * 
 * Implements hook_patterns().
 * 
 * @param string $data
 * 
 */
function system_patterns($data) {

  $files = array('modules/system/system.admin.inc');

  //Function not run through form. @TO-DO: create export function
  $actions['modules'] = array('descr' => t('Enable/Disable Modules'),
    PATTERNS_CREATE => array('modules_enable'),
    PATTERNS_DELETE => array('modules_disable'),
    PATTERNS_FILES  => $files,
  );

  //Function not run through form. @TO-DO: create export function
  $actions['theme'] = array('descr' => t('Enable (and set default)/Disable Themes'),
    PATTERNS_CREATE => array('set_active_theme'),
    PATTERNS_DELETE => array('set_active_theme'),
    PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'system_patterns_export_all_themes', ),
    PATTERNS_FILES  => $files,
  );

  // Determine necessary forms for action 'form' based on the data.
  $forms = array();
  if ($data) {
    if ($data['tag'] == 'form' && isset($data['form_id'])) {
      $forms[] = $data['form_id'];
    }
  }

  $actions['form'] = array('descr' => t('Submit Custom Forms'),
    PATTERNS_CREATE => $forms,
    PATTERNS_FILES  => $files,
  );

  //Function not run through form.
  $actions['call_php_func'] = array('descr' => t('Call PHP Functions'),
    PATTERNS_CREATE => array('call_php_func_form'),
  );

  //Function not run through form.
  $actions['variables'] = array('descr'  => t('Set/Modify/Delete System Variables'),
    PATTERNS_CREATE => array('variables_execute'),
    PATTERNS_MODIFY => array('variables_execute'),
    PATTERNS_DELETE => array('variables_execute'),
    PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'system_patterns_export_all_variables', ),
  );
  return $actions;

}


/**
 * @TO-DO!
 * Returns a set of PATTERNS_MODIFY actions with the whole set of variables
 * currently stored in the system.
 *
 * @param string $args
 * @param string $result
 * @return array $actions
 */
/*function system_patterns_export_all_modules($args = NULL, &$result = NULL) {
  @TO-DO
}*/

/**
 * @TO-DO!
 * Returns a set of PATTERNS_MODIFY actions with the whole set of themes
 * currently stored in the system.
 *
 * @param string $args
 * @param string $result
 * @return array $actions
 */
function system_patterns_export_all_themes($args = NULL, &$result = NULL) {
  //Retrieve the current list of themes and the default and admin values
  $themes = list_themes();
  $default_theme = variable_get('theme_default');
  $admin_theme = variable_get('admin_theme');
  $result = array();
  
  foreach ($themes as $theme) {
    //Prepare date for modify operation
    $data = array('tag' => 'theme');
    $data['value'] = $theme->name;
    $data['status'] = $theme->status;
    $data['default'] = ($theme->name == $default_theme);
    $data['admin'] = ($theme->name == $admin_theme);
    $action = array(PATTERNS_MODIFY => $data);
    array_push($result, $action);
  }
  return $result;
}

/**
 *
 * Returns a set of PATTERNS_CREATE or PATTERNS_MODIFY actions with 
 * the whole set of variables currently stored in the system.
 *
 * @param string $args
 * @param string $result
 * @return array $actions
 */
function system_patterns_export_all_variables($args = NULL, &$result = NULL) {
  //Get all the currently stored variables
  $query = db_select('variable', 'v')->extend('TableSort');
  $query->fields('v', array('name', 'value'));
  $qresult = $query->execute();
  
  //Got through all the variables and prepare set of actions according 
  //to the type of export process. In this case both cases have the same syntax
  if ($args['type'] == PATTERNS_CREATE ||PATTERNS_MODIFY) {
    $actions = array($args['type'] => array('tag' => 'variables'));
    foreach ($qresult as $row) {
      $name = $row->name;
      $value = variable_get($name);
      //Apply htmlspecialchars() function recursively to keep the same value input by the user
      _htmlspecialchars_decode($value);
      $action = array('name' => $name, 'value' => $value);
      array_push($actions[$args['type']], $action);
    }
  }
    
  $result = array($actions);
  return $result;
}


/**
 * @TO-CHECK
 * 
 * Prepare data for processing
 * 
 * @param string $action
 * @param string $tag
 * @param array $data
 * 
 */
function system_patterns_prepare($action, $tag, &$data) {
  if ($tag == 'theme') {
    if ($data['value']) {
      $data['theme_default'] = $data['value'];
    }
  }
  elseif ($tag == 'modules') {
    // Make a <modules>modulename</modules> style tag work
    if (is_string($data) || (isset($data['value']) && !isset($data[0]))) {
      $data = array($data);
    }

    // Ensure proper data format for cases when <modules> tag contains
    // only single <module> tag.
    if (!empty($data['module']) && is_string($data['module'])) {
      $data[0]['value'] = $data['module'];
      unset($data['module']);
    }

    foreach ($data as &$item) {
      // Ensure that modules with tags like <module>modulename</module>
      // are represented as an array instead of a string
      if (is_string($item)) {
        $item = array('value' => $item);
      }
    }

    // proccess alias for delete
    if (isset($data['disable']) && !isset($data['delete'])) {
      $data['delete'] = $data['disable'];
    }
    unset($data['disable']);
  }
  elseif ($tag == 'form') {
  }
  else {
    switch ($tag) {
      case 'variable':
        // Turn <variable>value</variable> style tag function the same as <variables>
        $tag = 'variables';
        $data = array($data);
      case 'variables':
        // Make a <variables>modulename</variables> style tag work

        if (!$data[0]) {
          if ($data['variable'] && isset($data['variable'][0])) {
            $data = $data['variable'];
            unset($data['variable']);
          }
          elseif ($data['variable'] && isset($data['variable']['value'])) {
            $data[0] = $data['variable'];
            unset($data['variable']);
          }
          else {
            $temp = $data;
            $data[0] = $temp;
            unset($data['id'], $data['value'], $data['delete']);
          }
        }

        for ($i = 0, $total = count($data); $i < $total; $i++) {
          $item = &$data[$i];

          if (!isset($item['value']) && (!isset($item['delete']) || !$item['delete'])) {
            $value = $item;
            unset($value['name']);
            $item = array('name' => $item['name'], 'value' => $value);
          }

        }

        break;
    }
  }
  // TODO: other tags

  return patterns_results();
}


/**
 *
 * @param string $action Type of action being executed
 * @param string $tag Type of tag to be validated
 * @param array $data Associative array containing the data action processed from the pattern
 *
 * @return mixed through patterns_results($status, $msg, $result) function. Status of the operation,
 * error messages and semantic warnings through $result
 */
function system_patterns_validate($action, $tag, &$data) {
  $result = array();
  $status = PATTERNS_SUCCESS;
  $msg = '';

  /*
   * Syntactic validation:
  * - Syntactic warnings: They refer to wrong grammar statements that will not
  * provoke any execution error. Ex.: non-interpretable attributes.
  * - Syntactic errors: They refer to wrong grammar statements that will provoke
  * an execution error. Ex.: misspelling in required attribute.
  *
  */
  switch ($tag) {
    case 'modules':
        switch ($action) {
        case PATTERNS_CREATE:
        case PATTERNS_DELETE:
          //In this case even the delete case is the same because the 'value' attribute is added by
          //the hook_prepare(). We will keep the code (switch with same cases) like this for future extendibility reasons
          $mandatory_attributes = array('value');
          //Go through all the variables to check if they have the mandatory attributes
          foreach ($data as $module) {
            if (!_patterns_has_all_mandatory_attributes($module, $mandatory_attributes, $msg)) {
              return patterns_results(PATTERNS_ERR, $msg);
            }
          }
          //In this case there are not optional attributes, so we can also use this set as interpretable attributes. Go through all variables
          foreach ($data as $module) {
            if(_patterns_has_uninterpretable_attributes($module, $mandatory_attributes, $msg)) {
              $status = PATTERNS_WARN;
            }
          }
          break;
        default:
          $msg = t('Action %action is uncompatible for tag %tag.', array('%action' =>$action, '%tag' =>$tag,));
          return patterns_results(PATTERNS_ERR, $msg);
      }
      break;
    case 'theme':
      //@TO-DO
      break;
    case 'form':
      //Original validation ported to syntactic case as error
      if (!isset($data['form_id'])) {
       $msg = t('"form_id" is missing.');
       return patterns_results(PATTERNS_ERR, $msg);
      }
      // Attempt to load required include file from menu.
      list($menu, $masks) = menu_router_build();
      foreach ($menu as $item) {
       if (isset($item['page arguments'][0]) && $item['page arguments'][0] == $data['form_id']
           && !empty($item['include file'])) {
         $data['include'] = $item['include file'];
         break;
       }
      }
      
      if (!empty($data['include']) && is_file($data['include'])) {
       require_once $data['include'];
      }
      elseif (is_array($data['include'])) {
       $data['include'] = drupal_get_path('module', $data['include']['module']) . '/' . $data['include']['file'];
      }
      
      if (!function_exists($data['form_id'])) {
       if (!empty($data['module']) && is_string($data['module'])) {
         $modules = module_list();
         if (in_array($data['module'], $modules)) {
           // try most common include file names
           module_load_include('inc', $data['module']);
           module_load_include('inc', $data['module'], $data['module'] . '.admin');
           module_load_include('inc', $data['module'], $data['module'] . '.page');
         }
       }
      }
      if (!function_exists($data['form_id'])) {
       $msg = t("Couldn't load the form %form. Check if all required modules are enabled and try to define 'include' or 'module' for this action.", array('%form' => $data['form_id']));
       return patterns_results(PATTERNS_ERR, $msg);
      }
      break;
    case 'call_php_func':
      //Original validation ported to syntactic case as error
      if (empty($data['function'])) {
       $msg = t("A function is required for this tag");
       return patterns_results(PATTERNS_ERR, $msg);
      }elseif (!empty($data['type']) && empty($data['module'])) {
       $msg = t("If you specify a type you must specify a module. See the documentation for module_load_include.");
       return patterns_results(PATTERNS_ERR, $msg);
      }elseif (empty($data['type']) && !empty($data['module'])) {
       $msg = t("If you specify a module you must specify a type. See the documentation for module_load_include.");
       return patterns_results(PATTERNS_ERR, $msg);
      }elseif (!empty($data['filepath']) && !file_exists($data['filepath'])) {
       $msg = t('The file that you specified does not exist: %file', array('%file' => $data['filepath']));
       return patterns_results(PATTERNS_ERR, $msg);
      }
      break;
    case 'variables':
      switch ($action) {
        case PATTERNS_CREATE:
        case PATTERNS_MODIFY:
        case PATTERNS_DELETE:
          //In this case even the delete case is the same because the 'value' attribute is added by
          //the hook_prepare(). We will keep the code (switch with same cases) like this for future extendibility reasons
          $mandatory_attributes = array('name', 'value');
          //Go through all the variables to check if they have the mandatory attributes
          foreach ($data as $variable) {
            if (!_patterns_has_all_mandatory_attributes($variable, $mandatory_attributes, $msg)) {
              return patterns_results(PATTERNS_ERR, $msg);
            }
          }
          //In this case there are not optional attributes, so we can also use this set as interpretable attributes. Go through all variables
          foreach ($data as $variable) {
            if(_patterns_has_uninterpretable_attributes($variable, $mandatory_attributes, $msg)) {
              $status = PATTERNS_WARN;
            }
          }
          break;
      }
      break;
  }

  /*
   * Semantic validation:
  * - Semantic warnings: They refer to the meaning of the pattern itself, and they
  * might provoke execution errors if they are not solved.
  *
  */
  switch ($tag) {
    case 'modules':
        $enabled_modules = module_list();
        switch ($action) {
        case PATTERNS_CREATE:
          //Check the modules are not already enabled
          foreach ($data as $module) {
            if (in_array($module['value'], $enabled_modules)) {
              $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT =>
                t('The module %mod is already enabled.', array('%mod' =>$module['value'],)));
            }  
          }
          break;
        case PATTERNS_DELETE:
          //Check the modules are not already disabled
          foreach ($data as $module) {
            if (!in_array($module['value'], $enabled_modules)) {
              $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT =>
                t('The module %mod is already disabled.', array('%mod' =>$module['value'],)));
            }  
          }
          break;
      }
      break;
    case 'theme':
      //@TO-DO
      //     // TODO: do this the Drupal7.x way
      //     $themes = system_rebuild_theme_data();
      //     if (!array_key_exists($data['theme_default'], $themes)) {
      //       $status = PATTERNS_ERR;
      //       $msg = t('%theme is not a valid theme.', array('%theme' => $data['theme_default']));
      //     }
      //   }
      break;
    case 'variables':
      switch ($action) {
        case PATTERNS_CREATE:
          //Check all the variables do not exist already in the system
          foreach ($data as $variable) {
            if (variable_get($variable['name'])) {
              $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT =>
                t('The variable %var already exists in the system.', array('%var' =>$variable['name'],)));
            }  
          }
          break;
        case PATTERNS_MODIFY:
        case PATTERNS_DELETE:
          //Check all the variables already exist already in the system
          foreach ($data as $variable) {
            if (!variable_get($variable['name'])) {
              $result[] = array(PATTERNS_WARNING_ELEMENT_UNDEFINED =>
                t('The variable %var does not exist in the system.', array('%var' =>$variable['name'],)));
            }  
          }
          break;
      }
      break;
  }
  return patterns_results($status, $msg, $result);
}


/**
 * 
 * Prepare for valid processing of this type of component
 * 
 * @param string $action
 * @param string $form_id
 * @param string $data
 * @param array $a
 * 
 */
function system_patterns_build($action, $form_id, &$data = NULL, &$a) {
  $status = PATTERNS_SUCCESS;
  $msg = '';
  $result = NULL;

  if ($form_id == 'set_active_theme') {
    $data['op'] = t('Save configuration'); // TODO: ??
    //module_load_include('inc', 'system', 'system.admin');
    // $result = $data;
  }
  elseif ($form_id == 'call_php_func_form') {
    $data['type'] = empty($data['type']) ? '' : $data['type'];
    $data['module'] = empty($data['module']) ? '' : $data['module'];
    $data['name'] = empty($data['name']) ? '' : $data['name'];
    $data['filepath'] = empty($data['filepath']) ? '' : $data['filepath'];

    if (empty($data['arguments'])) {
      $data['arguments'] = array();
    }
    elseif (!is_array($data['arguments'])) {
      $data['arguments'] = array($data['arguments']);
    }
    _call_php_func_include_files($data); // TODO: this works here?
    if (!is_callable($data['function'])) {
      $status = PATTERNS_ERR;
      $msg = t('The given function %func is not callable', array('%func' => $data['function']));
    }
    // $result = $data;
  } // TODO: other forms?
  else { // Running a user-provided form through tag 'form'.
    $form_state = $data['form_state_extras'];
    unset($data['form_state_extras']);
    unset($data['include']);
    $form_state['values'] = $data;
    $form_state['submitted'] = FALSE;
    if (!isset($form_state['storage'])) {
      $form_state['storage'] = NULL;
    }
    $result = $form_state;
  }
  return patterns_results($status, $msg, $result);
}

/**
 * 
 * Build a patterns actions and parameters
 * 
 * @param string $action
 * @param string $form_id
 * @param string $data
 * @param array $a
 * 
 */
function system_patterns_params($action, $form_id, &$data = NULL, &$a) {
  $result = NULL;

  if ($form_id == 'set_active_theme') {
    $result = array($data['theme_default'], isset($data['default']) ? TRUE : FALSE,
      isset($data['admin']) ? TRUE : FALSE);
  }
  elseif ($form_id == 'call_php_func_form') {
    $result = array($data['function'], $data['type'], $data['module'], $data['name'], $data['filepath'], $data['arguments']);
  }
  // TODO: other forms?

  /*
  elseif($form_id=='system_delete_date_format_type_form'){
    $data['confirm']=1;
    $result=array($data['date_type']);
  }*/
  else { // Running a user-provided form through tag 'form'.
    if (isset($data['args'])) {
      $params = $data['args'];
      unset($data['args']);
      $result = params;
    }
  }
  return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $result);
}

/**
 * 
 * Cleanup any global settings after the action runs
 * 
 * @param string $action
 * @param string $tag
 * @param array $data
 * 
 */
function system_patterns_cleanup($action, $tag, &$data) {
  if ($tag == 'modules') {
    menu_rebuild(); // TODO: only do this if there has been a module enabled/disabled
  }
  return patterns_results();
}


/**
 * Wraps the call to drupal_form_submit to enable modules. It enables automatically all the dependencies
 *
 * @param string $action  String containing the type of action. Currently, should be only 'modify'
 * @param array $data  Array of value variables containing the modules
 */
function modules_enable($action, &$data) {
  $modules = $data;
  foreach ($data['values'] as $module) {
    module_enable(array($module['value']), TRUE);
  }
}

/**
 * Wraps the call to drupal_form_submit to disable modules. It disables automatically all the dependencies
 *
 * @param string $action  String containing the type of action. Currently, should be only 'modify'
 * @param array $data  Array of value variables containing the modules
 */
function modules_disable($action, &$data) {
  $modules = $data;
  foreach ($data['values'] as $module) {
    module_disable(array($module['value']), TRUE);
  }
}

/**
 * Wraps the call to drupal_form_submit for setting the values of the variables
 * Goes through all the expected values from the form and prepare them for the query
 *
 * @param string $action  String containing the type of action. Currently, should be only 'modify'
 * @param array $data  Array of pairs variable name/value
 */
function variables_execute($action, &$data) {
  foreach ($data['values'] as $variable) {
    $var = variable_get($variable['name'], NULL);
    if (is_array($var)) {
      //Make sure we don't lose parts of the array that were not defined by pattern's action
      $var = array_merge($var, $variable['value']);
      variable_set($variable['name'], $var);
    }
    else {
      variable_set($variable['name'], $variable['value']);
    }
  }
}

/**
 * 
 * @TO-CHECK
 * 
 * @param unknown $form
 * @param unknown $form_state
 * @param unknown $theme_default
 * @param unknown $default
 * @param unknown $admin
 * @return multitype:multitype:unknown
 */
function set_active_theme($form, $form_state, $theme_default, $default, $admin) {
  return array(
    '#theme' => array(
      'theme_default' => $theme_default,
      'default' => $default,
      'admin' => $admin
    ),
  );
}

/**
 * @TO-CHECK
 * 
 * @param unknown $form
 * @param unknown $form_state
 */
function set_active_theme_submit($form, &$form_state) {
  $values = $form['#theme'];
  $theme = $values['theme_default'];
  $default = $values['default'];
  $admin = $values['admin'];
  patterns_system_theme_default($theme, $default, $admin);

}

/**
 * @TO-CHECK
 * 
 * Modified version of system_theme_default
 * 
 * @param unknown $theme
 * @param unknown $default
 * @param unknown $admin
 */
function patterns_system_theme_default($theme, $default, $admin) {
  // Get current list of themes.
  $themes = list_themes();

  // Check if the specified theme is one recognized by the system.
  if (!empty($themes[$theme])) {
    // Enable the theme if it is currently disabled.
    if (empty($themes[$theme]->status)) {
      // NOTE: this assumes sites/default/css exists.
      theme_enable(array($theme));
    }
    if ($default) {
      if ($admin) {
        // Set the admin theme.
        variable_set('admin_theme', $theme);
      }
      else {
        // Set the default theme.
        variable_set('theme_default', $theme);
      }
    }

    // Rebuild the menu. This duplicates the menu_rebuild() in theme_enable().
    // However, modules must know the current default theme in order to use
    // this information in hook_menu() or hook_menu_alter() implementations,
    // and doing the variable_set() before the theme_enable() could result
    // in a race condition where the theme is default but not enabled.
    menu_rebuild();
  }

}

/**
 * @TO-CHECK
 * 
 * @param unknown $form
 * @param unknown $form_state
 * @param unknown $func
 * @param unknown $type
 * @param unknown $module
 * @param unknown $name
 * @param unknown $filepath
 * @param unknown $args
 * @return multitype:multitype:unknown
 */
function call_php_func_form($form, $form_state, $func, $type, $module, $name, $filepath, $args) {
  return array(
    '#call_php_func' => array(
      'function' => $func,
      'type' => $type,
      'module' => $module,
      'name' => $name,
      'filepath' => $filepath,
      'arguments' => $args,
      ),
    );
}

/**
 * @TO-CHECK
 * 
 * @param unknown $form
 * @param unknown $form_state
 */
function call_php_func_form_submit($form, &$form_state) {
  $values = $form['#call_php_func'];
  _call_php_func_include_files($values);

  call_user_func_array($values['function'], $values['arguments']);
}

/**
 * @TO-CHECK
 * 
 * @param unknown $values
 */
function _call_php_func_include_files($values) {
  if ($values['type'] && $values['module']) {
    module_load_include($values['type'], $values['module'], $values['name']);
  }
  elseif ($values['filepath']) {
    require_once($values['filepath']);
  }
}

/**
 * 
 * Convert special HTML entities back to characters recursively.
 * 
 * @param string &$value
 */
function _htmlspecialchars_decode(&$value) {
  if (!is_array($value)) {
    $value = htmlspecialchars_decode($value);
  }else{
    foreach ($value as &$v) {
      _htmlspecialchars_decode($v);
    }
  }
}
