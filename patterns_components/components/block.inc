<?php
/*
 * TODO:
 * - Create a new tag for each element in the table.
 * - Create a new export function for each element in the table
 * - Re-implement completely the component, using custom functions to implement the changes
 * @file
 * Patterns component for blocks.
 */
function block_patterns($data) {

  $files = array('modules/block/block.admin.inc', 'modules/block/block.api.php');
  
  //All these functions are applied through custom functions
  
  $actions['block'] = array(
    PATTERNS_INFO => t('Create/Modify/Delete Blocks'),
    PATTERNS_CREATE => array('block_create'),
    PATTERNS_MODIFY => array('block_modify'),
    PATTERNS_DELETE => array('block_delete'),
    PATTERNS_FILES  => $files,
    PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'block_patterns_export_all_blocks', ),
  );
  
  $actions['block_custom'] = array(
      PATTERNS_INFO => t('Establish meta information relationship in custom modules'),
      PATTERNS_MODIFY => array('set_block_custom'),
      PATTERNS_FILES  => $files,
      PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'block_patterns_export_all_block_custom', ),
  );
  
  $actions['block_node_type'] = array(
      PATTERNS_INFO => t('Establish relationships between Blocks and Content Types'),
      PATTERNS_MODIFY => array('set_block_node_type'),
      PATTERNS_FILES  => $files,
      PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'block_patterns_export_all_block_node_types', ),
  );
  
  $actions['block_role'] = array(
      PATTERNS_INFO => t('Establish visibility relationships between Blocks and Roles'),
      PATTERNS_MODIFY => array('set_block_node_role'),
      PATTERNS_FILES  => $files,
      PATTERNS_EXPORT => array(PATTERNS_EXPORT_ALL => 'block_patterns_export_all_block_roles', ),
  );

  return $actions;

}

/**
 *
 * Returns a set of PATTERNS_CREATE or PATTERNS_MODIFY actions
 * with the whole set of blocks currently stored in the system.
 *
 * @param array $args  Set of arguments received from the form.
 * @param string $result  Current set of actions for the whole process.
 *
 * @return array $result  Set of actions after performing the changes.
 */
function block_patterns_export_all_blocks($args = NULL, &$result = NULL) {
  $blocks = _block_rehash();
  $result = array();

  //Prepare the set of actions according to the option selected 
  //TODO: The switch can be removed if both finally export the same fields. Keep temporarily until final cleanup
  switch ($args['type']) {
    case PATTERNS_CREATE :
      //Got through all the blocks and prepare set of actions.
      foreach ($blocks as $block) {
        $data = array('tag' => 'block');
        foreach ($block as $key => $value) {
          //In the case of modify, we discard the ID and the info (set through block_custom)
          if ($key != 'bid' && $key != 'info') {
            $data[$key] = $value;
          }
        }
        $action = array(PATTERNS_CREATE => $data);
        array_push($result, $action);
      }
      break;
    case PATTERNS_MODIFY :
      //Got through all the blocks and prepare set of actions.
      foreach ($blocks as $block) {
        $data = array('tag' => 'block');
        foreach ($block as $key => $value) {
          //In the case of modify, we discard only the info (set through block_custom)
          if ($key != 'bid' && $key != 'info') {
            $data[$key] = $value;
          }
        }
        $action = array(PATTERNS_MODIFY => $data);
        array_push($result, $action);
      }
      break;
  }
  return $result;
}

/**
 *
 * Returns a set of PATTERNS_MODIFY actions with the metadata stored
 * for all the custom blocks currently stored in the system.
 *
 * @param array $args  Set of arguments received from the form.
 * @param string $result  Current set of actions for the whole process.
 *
 * @return array $result  Set of actions after performing the changes.
 */
function block_patterns_export_all_block_custom($args = NULL, &$result = NULL) {
  $result = array();
  $block_custom_metadata = db_query('SELECT * from block_custom');

  //In this case the export is only as a set of modifys
  foreach ($block_custom_metadata as $metadata) {
    $data = array('tag' => 'block_custom');
    foreach ($metadata as $key => $value) {
      $data[$key] = $value;
    }
    $action = array(PATTERNS_MODIFY => $data);
    array_push($result, $action);
  }
  return $result;
}

/**
 *
 * Returns a set of PATTERNS_MODIFY actions with the relationship between blocks
 * and node types currently stored in the system.
 *
 * @param array $args  Set of arguments received from the form.
 * @param string $result  Current set of actions for the whole process.
 *
 * @return array $result  Set of actions after performing the changes.
 */
function block_patterns_export_all_block_node_types($args = NULL, &$result = NULL) {
  $result = array();
  $block_node_types = db_query('SELECT * from block_node_type');

  //In this case the export is only as a set of modifys
  foreach ($block_node_types as $node_type) {
    $data = array('tag' => 'block_node_type');
    foreach ($node_type as $key => $value) {
      $data[$key] = $value;
    }
    $action = array(PATTERNS_MODIFY => $data);
    array_push($result, $action);
  }
  return $result;
}


/**
 *
 * Returns a set of PATTERNS_MODIFY actions with the relationship between blocks
 * and roles currently stored in the system.
 *
 * @param array $args  Set of arguments received from the form.
 * @param string $result  Current set of actions for the whole process.
 *
 * @return array $result  Set of actions after performing the changes.
 */
function block_patterns_export_all_block_roles($args = NULL, &$result = NULL) {
  $result = array();
  $block_node_roles = db_query('SELECT * from block_role');

  //In this case the export is only as a set of modifys
  foreach ($block_node_roles as $role) {
    $data = array('tag' => 'block_role');
    foreach ($role as $key => $value) {
      //Transform the rid into the role name
      if ($key == 'rid') {
        $role = user_role_load($value);
        $data['role'] = $role->name;
      }else{
        $data[$key] = $value;
      }
    }
    $action = array(PATTERNS_MODIFY => $data);
    array_push($result, $action);
  }
  return $result;
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
function block_patterns_validate($action, $tag, &$data) {
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
    case 'block':
      switch ($action) {
        case PATTERNS_CREATE:
          //Check mandatory fields
          $mandatory_attributes = array('module', 'delta', 'title');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //Define interpretable attributes
          $interpretable_attributes = array('module', 'delta', 'title', 'theme', 'status', 'weight', 'region',
            'custom', 'visibility', 'pages', 'cache');
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_MODIFY:
          //Check mandatory fields
          $mandatory_attributes = array('module', 'delta', 'theme');
          if (!_patterns_has_all_mandatory_attributes($data, $mandatory_attributes, $msg)) {
            return patterns_results(PATTERNS_ERR, $msg);
          }
          //Define interpretable attributes
          $interpretable_attributes = array('module', 'delta', 'title', 'theme', 'status', 'weight', 'region',
            'custom', 'visibility', 'pages', 'cache');
          if(_patterns_has_uninterpretable_attributes($data, $interpretable_attributes, $msg)) {
            $status = PATTERNS_WARN;
          }
          break;
        case PATTERNS_DELETE:
          //@TO-DO
          break;
      }
      break;
    case 'block_custom':
      //The only available action for block_custom is modify, rest are considered syntactical errors
      if ($action == PATTERNS_MODIFY) {

      }else{
        $msg = t('Action %action is uncompatible for tag %tag.', array('%action' =>$action, '%tag' =>$tag,));
        return patterns_results(PATTERNS_ERR, $msg);
      }
      break;
    case 'block_node_type':
      switch ($action) {
        case PATTERNS_CREATE:
          //@TO-DO
          break;
        case PATTERNS_DELETE:
          //@TO-DO
          break;
      }
      break;
    case 'block_role':
        switch ($action) {
        case PATTERNS_CREATE:
          //@TO-DO
          break;
        case PATTERNS_DELETE:
          //@TO-DO
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
    case 'block':
      switch ($action) {
        case PATTERNS_CREATE:
          //Create semantic warning if the combination of module+delta already exists
          if (_block_patterns_module_delta_exists($data['module'], $data['delta'])) {
            $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT =>
                t('A block defined by module %module with delta %delta already exists in the system.',
                    array('%module' =>$data['module'], '%delta' =>$data['delta'])));
          }
          //We raised a semantic error if the theme does not exist
          if (isset($data['theme']) && !array_key_exists($data['theme'], list_themes())) {
            $result[] = array(PATTERNS_WARNING_UNMET_DEPENDENCY =>
                t('The theme %theme is not currently installed in the system.', array('%theme' =>$data['theme'],)));
          }
          break;
        case PATTERNS_MODIFY:
          //Create semantic warning if the combination of module+delta does not exist
          if (!_block_patterns_module_delta_exists($data['module'], $data['delta'])) {
            $result[] = array(PATTERNS_WARNING_ALREADY_DEFINED_ELEMENT =>
                t('There are not any blocks defined by module %module whose delta is %delta in the system.',
                    array('%module' =>$data['module'], '%delta' =>$data['delta'])));
          }
          //We raised a semantic error if the theme does not exist
          if (!array_key_exists($data['theme'], list_themes())) {
            $result[] = array(PATTERNS_WARNING_UNMET_DEPENDENCY =>
                t('The theme %theme to update is not currently installed in the system.', array('%theme' =>$data['theme'],)));
          }
          break;
        case PATTERNS_DELETE:
          //@TO-DO
          break;
      }
      break;
    case 'block_custom':
      //@TO-DO
      break;
    case 'block_node_type':
      switch ($action) {
        case PATTERNS_CREATE:
          //@TO-DO
          break;
        case PATTERNS_DELETE:
          //@TO-DO
          break;
      }
      break;
    case 'block_role':
      switch ($action) {
        case PATTERNS_CREATE:
          //@TO-DO
          break;
        case PATTERNS_DELETE:
          //@TO-DO
          break;
      }
      break;
  }
  return patterns_results($status, $msg, $result);
}


/**
 *
 * Wraps several calls to several drupal_form_submit to create a block.
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function block_create($form_id, &$form_state) {
  //We will need to make an insertion similar to the one in block_add_block_form_submit but 
  //in a simplified way. At this point all the mandatory values have been checked, and 
  //default values are provided for the optional attributes in case they are not provided
  $query = db_insert('block')
    ->fields(array('visibility', 'pages', 'custom', 'title', 'module', 'theme', 'status', 'weight', 'delta', 'cache', 'region'));
  $query->values(array(
      'visibility' => (isset($form_state['values']['visibility'])) ? (int) $form_state['values']['visibility']: 1,
      'pages' => (isset($form_state['values']['pages'])) ? trim($form_state['values']['pages']): '',
      'custom' => (isset($form_state['values']['custom'])) ? (int) $form_state['values']['custom']: 0,
      'title' => $form_state['values']['title'],
      'module' => $form_state['values']['module'],
      'theme' => (isset($form_state['values']['theme'])) ? $form_state['values']['theme']: variable_get('theme_default', 'bartik'),
      'status' => (isset($form_state['values']['status'])) ? $form_state['values']['status']: 0,
      'weight' => (isset($form_state['values']['weight'])) ? $form_state['values']['weight']: 0,
      'delta' => $form_state['values']['delta'],
      'cache' => (isset($form_state['values']['cache'])) ? $form_state['values']['cache']: -1,
      'region' => (isset($form_state['values']['region'])) ? $form_state['values']['region']: -1,
  ));
  $query->execute();

  return patterns_results(PATTERNS_SUCCESS, $msg);
}

/**
 *
 * Wraps several calls to several drupal_form_submit to update the settings of a block.
 *
 * @param string $form_id  String containing the form ID. In the case of custom functions the value is empty.
 * @param array $form_state  Set of values after parsing the action.
 */
function block_modify($form_id, &$form_state) {
  //We can use the all the optional fields provided by the pattern after removing the ones that compound the key
  $fields_to_remove = array('tag' => $form_state['values']['tag'], 'module' => $form_state['values']['module'] , 
      'delta' => $form_state['values']['delta'], 'theme' => $form_state['values']['theme']);
  $fields = array_diff_key($form_state['values'], $fields_to_remove);
  
  $query = db_update('block')
  ->fields($fields)
  ->condition('module', $form_state['values']['module'])
  ->condition('delta', $form_state['values']['delta'])
  ->condition('theme', $form_state['values']['theme'])
  ->execute();
  
  return patterns_results(PATTERNS_SUCCESS, $msg);
}


// // returns arguments needed for the forms when processing automatically
// // $loop is set if we want to extract all values
// function block_patterns_get_arguments($action, $tag, $form_id, &$loop = FALSE) {
//   // gives one dataset as an example how it looks like, while extract_all
//   // extracts all
//   if ($tag == 'block' && $action == PATTERNS_MODIFY) {
//     $block_list = _block_rehash();

//     if ($form_id == 'block_admin_display_form') {
//       $blocks = array();
//       foreach ($block_list as $key => $block) {
//         array_push($blocks, $block);
//       }
//       foreach (array_keys(list_themes()) as $theme) {
//         return array($blocks, $theme);
//       }
//     }
//     elseif ($form_id == 'block_admin_configure') {
//       foreach ($block_list as $key => $block) {
//         return array($block['module'], $block['delta']);
//       }
//     }

//     return array();
//   }
//  /* elseif ($tag == 'block_extract_all') {
//     $loop = TRUE;
//     $block_list = _block_rehash();
//     $return = array();

//     if ($form_id == 'block_admin_display_form') {
//       $blocks = array();
//       foreach ($block_list as $key => $block) {
//         array_push($blocks, $block);
//       }
//       foreach (array_keys(list_themes()) as $theme) {
//         array_push($return, array($blocks, $theme));
//       }
//     }
//     elseif ($form_id == 'block_admin_configure') {
//       foreach ($block_list as $key => $block) {
//         array_push($return, array($block['module'], $block['delta']));
//       }
//     }

//     return $return;
//   }*/
// }

// // Prepare data for processing
// function block_patterns_prepare($action, $tag, &$data) {
//   $defaults = array(
//     'id'          => NULL, // May be a combo of module+delta
//     'module'      => NULL,
//     #'delta'       => NULL, // Always use isset with delta, because 0 is a legal value
//     'description' => NULL,
//     'info'        => NULL,
//     'pages'       => NULL,
//   );
//   // Initialize the expected attributes to avoid PHP warnings. Makes later logic easier.
//   $data = array_merge($defaults, (array) $data);

//   if ($tag == 'block') {

//     // Alias.
//     if ($data['description']) {
//       $data['info'] = $data['description'];
//       unset($data['description']);
//     }

//     // Get the module and delta from the id, if provided.
//     if ($data['id']) {
//       $split = strpos($data['id'], '-');
//       if ($split === FALSE) {
//         // NOTE: No warning needed.
//       }
//       else {
//         $data['module'] = substr($data['id'], 0, $split);
//         $data['delta'] = substr($data['id'], $split + 1);
//       }

//       unset($data['id']);
//     }
//     // Block description for custom blocks is always unique so we can try to get delta by looking up info field
//     elseif ($data['info']) {
//       $delta = db_query('SELECT delta FROM {block} bl INNER JOIN {block_custom} bo ON bl.delta = bo.bid AND bl.module = :module WHERE bo.info = :info', array('module' => 'block', 'info' => $data['info']))->fetchField();
//       if ($delta) {
//         $data['delta'] = $delta;
//         $data['module'] = 'block';
//       }
//     }
//     else {
//       // TODO: Error: you must provide either id or info?
//     }

//     // The info field is needed for deletion.
//     if ($action === PATTERNS_DELETE && !$data['info'] && $data['module'] == 'block' && $data['delta']) {
//       $data['info'] = db_query('SELECT info FROM {block_custom} WHERE bid = :delta', array('delta' => $data['delta']))->fetchField();
//       // TODO: check if it exists?
//     }

//     // Split the pages array into a multi-line string.
//     if ($data['pages'] && is_array($data['pages'])) {
//       $pages = implode("\r\n", $data['pages']);
//       $data['pages'] = str_replace('[front]', '<front>', $pages);
//     }

//     $themes = array_keys(list_themes());

//     // If the regions field is missing, assign -1 to every theme.
//     if (!isset($data['regions']) or !is_array($data['regions'])
//       or empty($data['regions'])) {
//       $data['regions'] = array_combine($themes, array_fill(0, count($themes), '-1'));
//     }
//     else {
//       // Replace empty or missing regions with "-1"
//       foreach ($themes as $theme) {
//         if (!isset($data['regions'][$theme]) or empty($data['regions'][$theme])) {
//           $data['regions'][$theme] = -1;
//         }
//       }
//       // TODO: check if the regions are all valid?
//     }

//     // This must be a custom block.
//     if (!($data['module'] && isset($data['delta'])) && $data['info']) {
//       $data['module'] = 'block';
//     }

//   }

//   return patterns_results();
// }

// // Validate the values for an action before running the pattern
// function block_patterns_validate($action, $tag, &$data) {
//   $status = PATTERNS_SUCCESS;
//   $msg = '';

//   if ($tag == 'block') {
//     if (!($data['module'] && isset($data['delta'])) && !$data['info']) {
//       $status = PATTERNS_ERR;
//       $msg = t('Missing required &lt;module&gt;, &lt;delta&gt; tags. Possibly malformed &lt;id&gt; tag could be the problem too. If creating a new block, tag &lt;info&gt; is required.');
//     }
//     elseif ($action === PATTERNS_DELETE && $data['module'] != 'block') {
//       $status = PATTERNS_ERR;
//       $msg = t('Unable to delete non-block module blocks.');
//     }
//     elseif ($action === PATTERNS_MODIFY || $action === PATTERNS_DELETE) {
//       // Block must exist.
//       // TODO: support $data['info']
//       $block_count = db_query('SELECT COUNT(1) FROM {block} WHERE module = :module AND delta = :delta',
//         array('module' => $data['module'], 'delta' => $data['delta']))->fetchField();
//       if ($block_count == 0) {
//         $status = PATTERNS_ERR;
//         $msg = t('Block does not exist.');
//       }
//     }
//   }

//   return patterns_results($status, $msg);
// }
// // Return which callback functions to actually use.
// function block_patterns_callbacks($action, $tag, &$data) {
//   if ($tag == 'block') {
//     $keys = array('module', 'delta', 'theme', 'weight', 'region', 'status');
//     $diff = array_diff($data, $keys);

//     // TODO: document, make this clearer.
//     // Adding a new block: unknown id (or, module-delta) and a known info field.
//     if ($action === PATTERNS_CREATE && !($data['module'] && $data['delta']) && $data['info']) {
//       // $tag should be create
//       $result = array(
//           'block_add_block_form', // This will call block_admin_configure($form, $form_state, 'block', NULL);
//           'block_admin_display_form'
//         );
//     }
//     elseif ($action === PATTERNS_DELETE) { // Deleting a custom block.
//       $result = array('block_custom_block_delete');
//     }
//     elseif (empty($diff)) { // Only editing the position for one theme.
//       $result = array('block_admin_display_form'); // TODO: check _block_patterns_display_keys()
//     }
//     elseif ($action === PATTERNS_MODIFY) { // Editing a block
//       $result = array(
//           'block_admin_configure',
//           'block_admin_display_form'
//         );
//     }$field
//     else {
//       // TODO
//     }
//   }

//   return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $result);
// }

// // Prepare for valid processing of this type of component
// function block_patterns_build($action, $form_id, &$data, &$a) {
//   $status = PATTERNS_SUCCESS;
//   $msg = '';
//   $result = NULL;

//   module_load_include('inc', 'block', 'block.admin'); // TODO: is this needed?
//   // Fetch and sort blocks
//   $blocks = _block_rehash();
//   usort($blocks, '_block_compare');

//   if ($form_id == 'block_custom_block_delete') {
//     $data['op'] = t('Delete');
//     $data['confirm'] = 1;
//   }
//   elseif ($form_id == 'block_admin_display_form') {
//     if (isset($data['status']) && $data['status'] == 0) {
//       $data['region'] = -1; // BLOCK_REGION_NONE, modules/block/block.module
//     }

//     // Make sure delta is defined even for blocks that have just been created within the same action.
//     if ($data['module'] == 'block' && empty($data['delta'])) {
//       // We can do this because block descriptions are always unique.
//       $data['delta'] = db_query('SELECT bid FROM {block_custom} WHERE info = :info', array('info' => $data['info']))->fetchField();
//     }

//     $data = array($data['module'] . '_' . $data['delta'] => $data);
//   }

//   $result = $data;

//   return patterns_results($status, $msg, $result);
// }
// // Build a patterns actions and parameters
// function block_patterns_params($action, $form_id, &$data, &$a) {
//   if ($form_id == 'block_admin_configure') {
//     $result = array($data['module'], $data['delta']);
//   }
//   elseif ($form_id == 'block_admin_display_form') {
//     $blocks = _block_rehash(); // TODO: Haven't we done this before?
//     $check = end($data);

//     // Only return the block we are dealing with now
//     foreach ($blocks as $key => &$block) { // TODO: is there a function for this?
//       if ($block['module'] == $check['module'] && $block['delta'] == $check['delta']) {
//         $blocks = array(&$check);
//         break;
//       }
//     }
//     $result = array($blocks, $check['theme']);
//   }
//   elseif ($form_id == 'block_custom_block_delete') {
//     $result = array('block', $data['delta']);
//   }
//   return patterns_results(PATTERNS_SUCCESS, t('Execution successful'), $result);
// }

// // Cleanup any global settings after the action runs
// /*function block_patterns_cleanup($action, $tag, &$data) {
//   return patterns_results();
// }*/

/**
 * 
 * Checks the existance of a block with the given module and delta values
 * 
 * @param string $module  Module from which the block originates
 * @param string $delta  Unique ID for a block within a module
 * 
 * @return boolean  TRUE if there is a block matching both parameters, FALSE otherwise
 */
function _block_patterns_module_delta_exists ($module, $delta) {
  $query = db_select('block', 'b')
  ->fields('b')
  ->condition('module', $module)
  ->condition('delta', $delta)
  ->execute();

  return $query->rowCount() > 0;
}
