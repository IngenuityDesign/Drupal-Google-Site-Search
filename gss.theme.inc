<?php

/**
 * @file
 * Themeable functions for Google Site Search.
 */

/**
 * Process variables for gss-result.tpl.php.
 *
 * The $variables array contains the following arguments:
 * - $result
 * - $module
 *
 * @see gss-result.tpl.php
 */
error_reporting(E_ALL);
ini_set('display_errors', TRUE);
ini_set('display_startup_errors', TRUE);
function template_preprocess_gss_result(&$variables) {

  $result = $variables['result'];
  $variables['url'] = check_url($result['link']);
  $variables['title'] = $result['title'];

  // Check for existence. User search does not include snippets.
  $variables['snippet'] = isset($result['snippet']) ? $result['snippet'] : '';
  // Check for existence
  $variables['thumbnail_url'] = isset($result['thumbnail_url']) ? check_url($result['thumbnail_url']) : '';
  // Info variables
  $variables['info_split'] = array();
  $variables['info'] = '';
  $variables['show_info'] = FALSE;
  $variables['date'] = $result['date'] ? date('F j, Y', $result['date']) : false;
  $url = parse_url($result['link']);
  $url = trim($url['path'], '/');
  $languages = array_keys(language_list());
  $url_lang = NULL;
  foreach ($languages as $lang) {
    if ($lang . '/' === substr($url, 0, 3)) {
      $url_lang = substr($url, 0, 2);
      $url = substr($url, 3);
      break;
    }
  }

  // Get the Drupal system path for the URL alias
  preg_match('/node\/(\d+)/', $url, $matches);
  if (isset($matches[1])) {
    $nid = (int) $matches[1];
  }
  else {
    $system_path = drupal_lookup_path('source', $url, $url_lang);
    preg_match('/node\/(\d+)/', $system_path, $matches);
    $nid = isset($matches[1]) ? (int)$matches[1] : 0;
  }

  if ($nid && $node = node_load($nid)) {
    $user = user_load($node->uid);
    $user_name = l($user->name, 'user/' . $user->uid);
    $info = array(
      'type' => $node->type,
      'user' => $user_name,
      'date' => format_date($node->created, 'short'),
    );
      $variables['date'] = date('F j, Y', $node->created);
    $variables['info_split'] = $info;
    $variables['info'] = implode(' | ', $info);
    $variables['show_info'] = variable_get('gss_info', FALSE);
  } else {
      //not loading
  }
}

function gss_build_param_list($params) {
    $url = '?';
    foreach($params as $k=>$v) {
        $url .= sprintf("%s=%s&", $k, $v);
    }

    return rtrim($url, '&');
}

/**
 * Process variables for gss-results.tpl.php.
 *
 * The $variables array contains the following arguments:
 * - $result
 * - $module
 *
 * @see gss-results.tpl.php
 */
function template_preprocess_gss_results(&$variables) {

    $params = array();
    foreach( $_GET as $param => $value ) {
        $params[$param] = urlencode(trim($value));
    }

    unset($params['q']);

    $rel = $params;
    $rel['sort'] = 'relevance';

    $date = $params;
    $date['sort'] = 'date';
    $variables['sorting_date'] = gss_build_param_list($date);
    $variables['sorting_rel'] = gss_build_param_list($rel);

  $results = $variables['results'];
  $variables['sort'] = get_sorting_keyword();
  $variables['head'] = $results['head'];
  unset($results['head']);
  $variables['pager'] = $results['pager'];
  unset($results['pager']);
  $variables['search_results'] = '';
  foreach ($results as $entry) {
    $variables['search_results'] .= theme('gss_result', array('result' => $entry));
  }
}

