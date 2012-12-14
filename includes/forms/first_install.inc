<?php
/**
 * @file
 * Displays a form of necessary input configuration to the user.
 */

function patterns_first_install_page() {

  module_load_include('inc', 'patterns', 'includes/forms/modules');
  $out = drupal_get_form('patterns_first_install');
  return $out;
}


function patterns_first_install($form, &$form_state) {

  $intro1 = 'Patterns helps you to bypass the bottleneck of managing and automating site configuration. Start creating, and sharing patterns files in YAML, XML, or even PHP format.';
  
  $intro2 = 'You just installed Patterns, or an important update was found';
  
  //$intro.= 'This page gives you a brief guide to the main functionalities of Patterns and settings. TODO check which parsers are enabled, if components are enabled, path to drush, and other settings.';

  $form['welcome'] = array(
    '#type' => 'fieldset',
    '#title' => t('Welcome to Patterns!'),
  );

  $form['welcome']['intro'] = array(
    '#prefix' => '<div>',
    '#suffix' => '</div><br/>',
    '#markup' => t($intro1),
  );
  
  $resources = array();
  $resources['items'] = array(
    t('!documentation', array('!documentation' => l(t('Online documentation'), 'http://drupal.org/node/1464118'))),
    t('!settings', array('!settings' => l(t('Settings'), 'admin/patterns/settings'))),
  );

  $resources['title'] = t('Resources');
  $resources['type'] = 'ul';
  $resources['attributes'] = array();
  
  $form['welcome']['resources'] = array(
    '#markup' => theme_item_list($resources),
    '#suffix' => '<br>',
  );

  $form['welcome']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('OK!'),
    '#suffix' => '<div>' . t('By pressing OK this page will not be shown any more.') . '</div>',
  );
  
  $parsers = patterns_parser_build_formats_index();
  if (empty($parsers)) {
    $form['welcome']['parsers'] = array(
      '#prefix' => '<br/><strong>' . t('Warning:') . ' </strong>',
      '#markup' => t('It seems that you have not yet enabled any Patterns parser. You will not be able to load or run any pattern. Please visit the !modules page to enable at least one parser.', array('!modules' => l(t('modules'), 'admin/modules'))),
    );
  }
  
  $path = libraries_get_path('spyc') . '/spyc.php';

  // Valid in installation and run-time

  if (!file_exists($path)) {
    $form['welcome']['spyc'] = array(
      '#prefix' => '<br/><strong>' . t('Warning:') . ' </strong>',
      '#markup' => t('Spyc library for parsing YAML files not found. Please download the !spyc package and copy spyc.php into your libraries folder (e.g. sites/all/libraries/spyc/).',
                          array('!spyc' => l(t('spyc'), 'http://code.google.com/p/spyc/'))),
    );
  }
  
  
  $cm_path = libraries_get_path('codemirror-3.0');
  if (empty($cm_path) || !file_exists($cm_path)) {
  	$form['welcome']['cm'] = array(
      '#prefix' => '<br/><strong>' . t('Warning:') . ' </strong>',
      '#markup' => t('Codemirror 3.0 not found. Download !codemirror and copy it into your libraries folder (e.g. sites/all/libraries/codemirror-3.0/) to a have a better editor experience.',
                          array('!codemirror' => l(t('codemirror 3.0'), 'http://codemirror.net/'))),
  	);	
  }
   


  

  

  return $form;
}

function patterns_first_install_validate($form, &$form_state) {
  return TRUE;
}

function patterns_first_install_submit($form, &$form_state) {
  variable_set(PATTERNS_FIRST_INSTALL, FALSE);
  menu_rebuild();
}
