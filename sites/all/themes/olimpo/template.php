<?php


/**
 * Override or insert PHPTemplate variables into the templates.
 */
function olimpo_preprocess_page(&$vars) {
    $scripts = drupal_add_js();
    if (_olimpo_use_new_jquery()) {
        $jquery_path = _olimpo_get_jquery_path();
        $new_jquery = array($jquery_path => $scripts['core']['misc/jquery.js']);
        $scripts['core'] = array_merge($new_jquery, $scripts['core']);
        unset($scripts['core']['misc/jquery.js']);
    }
    $vars['scripts'] = drupal_get_js('header', $scripts);
    $vars['main_menu'] = olimpo_theme_menu(array('main-menu--02', 'js__main-menu--02', 'mq-small--hidden'));
    $vars['left_menu'] = olimpo_theme_menu(array('rwd__main-menu'));
}


function _olimpo_use_new_jquery() {
    return arg(0) != 'admin' && arg(1) != 'add' && arg(2) != 'edit' && arg(0) != 'panels' && arg(0) != 'ctools' && arg(0) != 'imagecrop' && arg(0) != 'batch';
    ;
}


function _olimpo_get_jquery_path() {
    return drupal_get_path('theme', 'olimpo') . base_path() . "js/jquery/jquery-2.0.3.min.js";
}


function olimpo_theme_menu($classes = array()) {
  $class = implode(' ', $classes);
  $menu = olimpo_get_header_menu();
  $output = '';
  foreach ($menu as $item) {
    $item_attributes = olimpo_is_menu_item_active($item) ? ' class="active"' : '';
    $path = base_path() . $item['path_alias'];
    $output .= "<li><a href=\"$path\"$item_attributes>{$item['name']}</a></li>";
  }
  return "<ul class=\"$class\">$output</ul>";
}


function olimpo_get_header_menu() {
  return array(
    array(
      'name' => 'Noticias',
      'path' => 'noticias',
      'path_alias' => 'noticias'
    ),
    array(
      'name' => 'Gimnasio',
      'path' => 'node/14',
      'path_alias' => 'gimnasio'
    ),
    array(
      'name' => 'Imágenes',
      'path' => 'imagenes',
      'path_alias' => 'imagenes'
    ),
    array(
      'name' => 'Palmarés',
      'path' => 'record-book',
      'path_alias' => 'palmares'
    )
  );
}


function olimpo_is_menu_item_active($item) {
  return arg(0) == $item['path'];
}


function olimpo_preprocess_node(&$variables) {
    switch($variables['node']->type){
    case 'image':
        $sql = 'SELECT n.* FROM {node} n WHERE n.nid IN (SELECT tn.nid FROM {term_node} tn WHERE tn.nid != %d AND tn.tid IN (SELECT tnd.tid FROM {term_node} tnd WHERE tnd.nid = %d)) ORDER BY n.nid ASC';
        $result = db_query($sql, $variables['node']->nid, $variables['node']->nid);
        $para = $siguiente = $anterior = FALSE;
        while(($res = db_fetch_object($result)) && !$para) {
            if($res->nid < $variables['node']->nid)
                $anterior = $res->nid;
            if($res->nid > $variables['node']->nid) {
                $siguiente = $res->nid;
                $para = TRUE;
            }
        }
        $variables['siguiente'] = $siguiente;
        $variables['anterior'] = $anterior;
        break;
    }
}

