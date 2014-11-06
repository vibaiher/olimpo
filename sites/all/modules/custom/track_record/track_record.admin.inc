<?php


function track_record_club_records_admin_page() {
  return drupal_get_form('track_record_add_club_record_form', track_record_get_club_information());
}


function track_record_get_medals_for_select() {
  $medals = array(
    '' => t('Nothing'),
    TRACK_RECORD_GOLD_MEDAL => t('Gold'),
    TRACK_RECORD_SILVER_MEDAL => t('Plate'),
    TRACK_RECORD_BRONZE_MEDAL => t('Bronze'),
  );

  return $medals;
}


function track_record_add_club_record_form($form_state, $club) {
  $form = array();

  $form['club'] = array(
    '#type' => 'hidden',
    '#value' => $club['id']
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title')
  );
  $form['medal'] = array(
    '#type' => 'select',
    '#title' => t('Medal'),
    '#options' => track_record_get_medals_for_select()
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );

  return $form;
}


function track_record_add_club_record_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  if ($values['medal'] != '') {
    db_query("INSERT INTO {track_record_club_records} (title, medal, club_id) VALUES ('%s', %d, %d)", $values['title'], $values['medal'], $values['club']);
  }
  else {
    db_query("INSERT INTO {track_record_club_records} (title, club_id) VALUES ('%s', %d)", $values['title'], $values['club']);
  }
}


function track_record_athletes_admin_page() {
  $athletes = track_record_get_athletes_information();
  $headers = array(t('Name'), t('Birthday'), t('Photo'));
  $rows = array();
  foreach ($athletes as $athlete) {
    $rows[] = array(
      l($athlete['name'], "admin/track_record/athletes/{$athlete['id']}"),
      date('d-m-Y', $athlete['birthday']),
      !is_null($athlete['photo_id']) ? t('Yes') : t('No')
    );
  }

  $content = '<h1>' . t('Athletes administration') . '</h1>';
  $content .= '<div class="add-form">' . drupal_get_form('track_record_add_athlete_form') . '</div>';
  $content .= theme('table', $headers, $rows);

  return $content;
}


function track_record_add_athlete_form() {
  $form = array(
    '#attributes' => array('enctype' => 'multipart/form-data')
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name')
  );
  $form['birthday'] = array(
    '#type' => 'date',
    '#title' => t('Birthday')
  );
  $form['photo'] = array(
    '#type' => 'file',
    '#title' => t('Photo')
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create athlete')
  );

  return $form;
}


function track_record_add_athlete_form_validate($form, $form_state) {
  if (!isset($_FILES['files']['tmp_name']['photo'])) {
    form_set_error('photo', t('You have to upload the file'));
  }
}


function track_record_add_athlete_form_submit($form, &$form_state) {
  $file = file_save_upload('photo', array(), file_directory_path() . '/track_record_images');
  $birthday = strtotime("{$form_state['values']['birthday']['year']}-{$form_state['values']['birthday']['month']}-{$form_state['values']['birthday']['day']}");
  db_query("UPDATE {files} SET status=%d WHERE fid=%d", FILE_STATUS_PERMANENT, $file->fid);
  db_query("INSERT INTO {track_record_athletes} (name, birthday, photo_id) VALUES ('%s', %d, %d)", $form_state['values']['name'], $birthday, $file->fid);
}


function track_record_athlete_admin_page($athlete_id) {
  $athlete = track_record_get_athlete_information($athlete_id);

  $content = drupal_get_form('track_record_update_athlete_form', $athlete);

  return $content;
}


function track_record_update_athlete_form($form_state, array $athlete) {
  $form = array(
    '#attributes' => array('enctype' => 'multipart/form-data')
  );
  $form['athlete_id'] = array(
    '#type' => 'hidden',
    '#value' => $athlete['id']
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $athlete['name']
  );
  $form['birthday'] = array(
    '#type' => 'date',
    '#title' => t('Birthday'),
    '#default_value' => array(
      'year' => date('Y', $athlete['birthday']),
      'month' => date('n', $athlete['birthday']),
      'day' => date('j', $athlete['birthday']),
    )
  );
  if (isset($athlete['photo'])) {
    $form['old_photo'] = array(
      '#type' => 'markup',
      '#value' => theme('track_record_athlete_photo', $athlete)
    );
  }
  $form['photo'] = array(
    '#type' => 'file',
    '#title' => t('New photo')
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update athlete')
  );

  $rows = array();
  $medal_options = track_record_get_medals_for_select();
  foreach ($athlete['records'] as $record) {
    $rows[] = array($record['title'], $medal_options[$record['medal']]);
  }

  $form['track_record'] = array(
    '#type' => 'markup',
    '#value' => theme('table', array(t('Title'), t('Medal')), $rows)
  );

  $form['add_track_record'] = array(
    '#type' => 'markup',
    '#value' => l(t('Add to track record'), "admin/track_record/athletes/{$athlete['id']}/records")
  );

  return $form;
}


function track_record_update_athlete_form_submit($form, &$form_state) {
  if (isset($_FILES['files']['tmp_name']['photo'])) {
    $file = file_save_upload('photo', array(), file_directory_path() . '/track_record_images');
    db_query("UPDATE {files} SET status=%d WHERE fid=%d", FILE_STATUS_PERMANENT, $file->fid);
  }
  $birthday = strtotime("{$form_state['values']['birthday']['year']}-{$form_state['values']['birthday']['month']}-{$form_state['values']['birthday']['day']}");
  if (isset($file) && isset($file->fid)) {
    db_query("UPDATE {track_record_athletes} SET name = '%s', birthday = %d, photo_id = %d WHERE id = %d", $form_state['values']['name'], $birthday, $file->fid, $form_state['values']['athlete_id']);
  }
  else {
    db_query("UPDATE {track_record_athletes} SET name = '%s', birthday = %d WHERE id = %d", $form_state['values']['name'], $birthday, $form_state['values']['athlete_id']);
  }
}

function track_record_athlete_records_admin_page($athlete_id) {
  $athlete = track_record_get_athlete_information($athlete_id);
  $content .= drupal_get_form('track_record_add_athlete_record_form', $athlete);

  drupal_set_title(t('Add entry to @name track record', array('@name' => $athlete['name'])));

  return $content;
}


function track_record_add_athlete_record_form($form_state, $athlete) {
  $form = array();

  $form['athlete'] = array(
    '#type' => 'hidden',
    '#value' => $athlete['id']
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title')
  );
  $form['medal'] = array(
    '#type' => 'select',
    '#title' => t('Medal'),
    '#options' => track_record_get_medals_for_select()
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#suffix' => t('or !cancel', array('!cancel' => l(t('Cancel'), 'admin/track_record/athletes/' . $athlete['id'])))
  );

  return $form;
}


function track_record_add_athlete_record_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  if ($values['medal'] != '') {
    db_query("INSERT INTO {track_record_athlete_records} (title, medal, athlete_id) VALUES ('%s', %d, %d)", $values['title'], $values['medal'], $values['athlete']);
  }
  else {
    db_query("INSERT INTO {track_record_athlete_records} (title, athlete_id) VALUES ('%s', %d)", $values['title'], $values['athlete']);
  }
  $form_state['redirect'] = 'admin/track_record/athletes/' . $athlete['id'];
}