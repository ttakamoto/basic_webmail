<?php
/**
 * @file
 * This file handles the message listing.
 */

/**
 * Display the listing of messages.
 *
 * @param array $form
 *   The form definition.
 * @param array $form_state
 *   The form values of the passed form.
 * @param string $folder
 *   The name of the folder to open. If no folder is specified, the default
 *   of 'INBOX' is used.
 *
 * @return array
 *   The formatted message listing.
 */
function basic_webmail_message_list($folder = 'INBOX') {
  if (empty($folder)) {
    $folder = 'INBOX';
  }
  $folder = str_replace('\\', '/', $folder);
  return drupal_get_form('basic_webmail_mail_list_form', $folder);
}


/**
 * Display the listing of messages.
 *
 * @param array $form
 *   The form definition.
 * @param array $form_state
 *   The form values of the passed form.
 * @param string $folder
 *   The name of the folder to open. If no folder is specified, the default
 *   of 'INBOX' is used.
 *
 * @return array
 *   The formatted message listing.
 */
function basic_webmail_mail_list_form($form, &$form_state, $folder = 'INBOX') {
  // Initialize the form for dealing with messages in bulk.
  $form = array();

  global $user;
  if (empty($user->data['basic_webmail_username'])) {
    $form['error'] = array(
      '#type' => 'item',
      '#markup' => t('You have not configured your settings to use this webmail system. Edit your !account and fill in the necessary fields in the "Basic webmail account settings" group.', array('!account' => l('account settings', 'user'))),
    );
    return $form;
  }

  // Set the title of the page to the current mailbox, if there is one.
  if (!empty($folder)) {
    drupal_set_title(t('Current folder: @title', array('@title' => $folder)));
  }

  // Load the custom CSS file.
  drupal_add_css(drupal_get_path('module', 'basic_webmail') . '/basic_webmail.css');

  // Get the list of folders.
  $folder_list = _basic_webmail_get_folder_list($folder);

  // Show the current mailbox.
  $form['mailbox_title'] = array(
    '#type' => 'item',
    '#title' => t('Current mailbox: !mailbox', array('!mailbox' => $user->data['basic_webmail_username'])),
  );

  // Prepare the list of folders for display above the message list.
  if (!empty($folder_list)) {
    $form['folders'] = array(
      '#type' => 'item',
      '#title' => t('Other folders in this mailbox:'),
      '#markup' => _basic_webmail_prepare_folder_list($folder_list, $folder),
    );
  }

  $form['actions_title'] = array(
    '#type' => 'item',
    '#title' => t('Actions:'),
  );

  // Add a link to a blank composition form.
  $form['comoposition'] = array(
    '#type' => 'item',
    '#markup' => l(t('Compose new message'), 'basic_webmail/sendmail') . '<hr />',
  );

  // Create the table header for the message listing.
  $header = _basic_webmail_create_message_list_header();

  // Update the saved message information.
  _basic_webmail_update_stored_message_list($folder);

  // Get the message list.
  $message_list = _basic_webmail_get_message_list_data($header, $user->uid, $folder);

  $form['message_list_title'] = array(
    '#type' => 'item',
    '#title' => t('Messages in this folder:'),
  );

  // Add the message list to the form.
  $form['message_list'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $message_list,
    '#empty' => t('You have no email in this mailbox.'),
  );

  // Add the operation select list to the form.
  $form['operation'] = array(
    '#type' => 'select',
    '#title' => t('With checked'),
    '#default_value' => 'read',
    '#options' => array(
      'delete' => t('Delete'),
      'read' => t('Mark read'),
      'unread' => t('Mark unread'),
      'copy_to' => t('Copy'),
      'move_to' => t('Move'),
    ),
  );

  // Add the select list with the list of folders.
  $form['folder_name'] = array(
    '#type' => 'select',
    '#title' => t('The folder to copy or move to'),
    '#options' => $folder_list,
  );

  // Add the button.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );

  // Save the current folder name for later use.
  $form['folder'] = array(
    '#type' => 'hidden',
    '#value' => $folder,
  );

  // Add the pager at the bottom of the form.
  $form['pager'] = array('#markup' => theme('pager'));

  return $form;
}

/*
 * Generate the block content for received/sent emails.
 *
 * @return string
 *   Returns a HTML content after beind parsed using theme function and the
 * specific template file.
 */
function _basic_webmail_block_content($num = 5) {
  return theme('basic_webmail_block_content_theme',_basic_webmail_retrieve_last_messages($num));
}

/**
 * Retrieve the content for block that list last # messages of INBOX/Sent
 * mailboxes
 *
 * @param int  $num
 *   number of messages of each mailbox.
 *
 * @return array
 *   The formatted message listing.
 */
function _basic_webmail_retrieve_last_messages($num = 5) {
  global $user;
  if (empty($user->data['basic_webmail_username'])) {
    $form['error'] = array(
      '#type' => 'item',
      '#markup' => t('You have not configured your settings to use this webmail system. Edit your !account and fill in the necessary fields in the "Basic webmail account settings" group.', array('!account' => l('account settings', 'user'))),
    );
    return $form;
  }

  // Get the list of folders.
  $inbox_folder = 'INBOX';
  $sent_folder = variable_get('basic_webmail_current_sent_folder', 'Sent Mail');
  
  // Verify if we are getting content from cached tables or retrieving from server.
  $last_pull = variable_get('basic_webmail_block_content_last_pull_timestamp', NULL);
  $is_caching = variable_get('basic_webmail_cache_block_content', 0);
  if ($is_caching == 0 || $last_pull == NULL || (time() - $last_pull > $is_caching)) {
    $folder_list = _basic_webmail_get_folder_list($inbox_folder);
    $sent_folder = variable_get('basic_webmail_current_sent_folder', 'Sent Mail');
  
    // Update the saved message information.
    _basic_webmail_update_stored_message_list($inbox_folder);
    _basic_webmail_update_stored_message_list($sent_folder);
  }
  
  // Create the table header for the message listing.
  $header = _basic_webmail_create_message_list_header();
  
  // Verify if filtering must be done
  $from = '';
  if (variable_get('basic_webmail_filter_recent_block_messages') == 1) {
    include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
    $profile_mail_address = _basic_webmail_is_user_profile_page();
    if($profile_mail_address != FALSE) {
      $from = $profile_mail_address;      
    }
  }
  
  // Get the message list.
  $inbox_message_list = _basic_webmail_get_message_list_data($header, $user->uid, $inbox_folder, $num, $from);
  $sent_message_list = _basic_webmail_get_message_list_data($header, $user->uid, $sent_folder, $num, $from);

  return array(
    'inbox' => $inbox_message_list,
    'sent' => $sent_message_list,   
    'is_filtering' => variable_get('basic_webmail_filter_recent_block_messages'),
    'from' => $from,  
  );
}


/**
 * Perform an action on a group of messages.
 *
 * @param array $form
 *   The form definition.
 * @param array $form_state
 *   The form values of the passed form.
 */
function basic_webmail_mail_list_form_submit($form, &$form_state) {
  // Find out what is to be done with the messages.
  $operation = $form_state['values']['operation'];
  // Filter out unselected messages.
  $messages = array_filter($form_state['values']['message_list']);

  if (isset($messages) && is_array($messages)) {
    // Connect to the server and retrieve a connection to the mailbox.
    include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
    global $user;
    $imap_resource = _basic_webmail_connect_to_server($user->uid, $form_state['values']['folder']);
    if (empty($imap_resource)) {
      $message = 'There was an error connecting to the mail server. Contact the system administrator and/or check the logs for more information.';
      _basic_webmail_report_error($message, array(), WATCHDOG_ERROR, TRUE);
      // @todo: figure out if this is necessary.
      // $form_state['redirect'] = $message;
    }

    switch ($operation) {
      case 'delete':
        _basic_webmail_mail_list_form_submit_delete($form_state, $messages, $imap_resource);
        break;

      case 'read':
        _basic_webmail_mail_list_form_submit_mark_read($form_state, $messages, $imap_resource);
        break;

      case 'unread':
        _basic_webmail_mail_list_form_submit_mark_unread($form_state, $messages, $imap_resource);
        break;

      case 'copy_to':
        _basic_webmail_mail_list_form_submit_copy_to($form_state, $messages, $imap_resource);
        break;

      case 'move_to':
        _basic_webmail_mail_list_form_submit_move_to($form_state, $messages, $imap_resource);
        break;
    }

    // Clean up.
    if (!imap_close($imap_resource)) {
      $message = 'There was an error closing the IMAP stream.';
      _basic_webmail_report_error($message, array(), WATCHDOG_WARNING);
    }
  }

  // Send the user back to the page they were looking at.
  $form_state['redirect'] = _basic_webmail_get_return_destination();
}


/**
 *  Private helper functions that do the main work of the module.
 */


/**
 * Retrieves the list of folders.
 *
 * @param string $folder
 *   The name of the folder to open. If no folder is specified, the default
 *   of 'INBOX' is used.
 *
 * @return array
 *   The folder listing.
 */
function _basic_webmail_get_folder_list($folder = 'INBOX') {
  // Connect to the server and retrieve a connection to the mailbox.
  include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
  global $user;
  $imap_resource = _basic_webmail_connect_to_server($user->uid, $folder);
  if ($imap_resource === FALSE) {
    $message = 'There was an error connecting to the mail server. Contact the system administrator and/or check the logs for more information.';
    _basic_webmail_report_error($message, array(), WATCHDOG_ERROR, TRUE);
    return FALSE;
  }

  // Get the list of folders from the server.
  $folders = imap_list($imap_resource, _basic_webmail_get_server_string(), '*');
  if (!empty($folders)) {
    natsort($folders);
    foreach ($folders as $folder_name) {
      $brief_folder_name = imap_utf7_decode(drupal_substr($folder_name, strpos($folder_name, '}') + 1, drupal_strlen($folder_name)));

      if ($brief_folder_name != $folder) {
        $folder_list[$brief_folder_name] = $brief_folder_name;
      }
    }
  }

  // Clean up.
  if (!imap_close($imap_resource)) {
    $message = 'There was an error closing the IMAP stream.';
    _basic_webmail_report_error($message, array(), WATCHDOG_WARNING);
  }
  
  // Setting the current Sent Mail folder since the INBOX folder is already default.
  variable_set('basic_webmail_current_sent_folder', 'Sent Mail');
  foreach ($folder_list as $value) {
    if (strpos($value, 'Sent') != FALSE) {
      variable_set('basic_webmail_current_sent_folder', $value);
    }
  }
  
  return $folder_list;
}


/**
 * Prepares the list of folders for display.
 *
 * @param string $folder
 *   The name of the folder to open. If no folder is specified, the default
 *   of 'INBOX' is used.
 *
 * @return string
 *   The folder listing markup for displaying.
 */
function _basic_webmail_prepare_folder_list($folder_list, $folder = 'INBOX') {
  // Initialize the variable.
  $output = '<br />';

  // Iterate through the list of folders.
  foreach ($folder_list as $folder_name) {
    // Get the name for the current folder.
    $brief_folder_name = imap_utf7_decode(drupal_substr($folder_name, strpos($folder_name, '}'), drupal_strlen($folder_name)));
    // Replaces the "/" char that gmails use to an \ for URL parse
    $brief_folder_name = str_replace('/', '\\', $brief_folder_name);
    // Add the folder to the list with a link.
    $output .= l($brief_folder_name, 'basic_webmail/list/' . $brief_folder_name) . '&nbsp; -- &nbsp;';
  }

  // Remove the final, unnecessary &nbsp; -- &nbsp;.
  $output = drupal_substr($output, 0, drupal_strlen($output) - 16);
  $output .= '<hr>';

  return $output;
}


/**
 * Create the table header for the message listing.
 *
 * @return array
 *   The table header for the message list.
 */
function _basic_webmail_create_message_list_header() {
  // Set the path to the icons.
  $icon_path = drupal_get_path('module', 'basic_webmail') . '/images/';

  // Specify the header information for the message list.
  return array(
    'message_unread' => array('data' => t('Read'), 'field' => 'message_unread'),
    'message_subject' => array('data' => t('Subject'), 'field' => 'message_subject'),
    'from_address' => array('data' => t('From'), 'field' => 'from_address'),
    'message_date' => array(
      'data' => t('Date/Time'),
      'field' => 'message_date',
      'sort' => 'desc',
    ),
  );
}


/**
 * Updates the saved message list information.
 *
 * @param string $folder
 *   The name of the folder to open. If no folder is specified, the default
 *   of 'INBOX' is used.
 */
function _basic_webmail_update_stored_message_list($folder = 'INBOX') {
  // Connect to the server and retrieve a connection to the mailbox.
  include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
  global $user;
  $imap_resource = _basic_webmail_connect_to_server($user->uid, $folder);
  if (empty($imap_resource)) {
    $message = 'There was an error connecting to the mail server. Contact the system administrator and/or check the logs for more information.';
    _basic_webmail_report_error($message, array(), WATCHDOG_ERROR, TRUE);
    return;
  }

  $mailboxes = imap_getsubscribed($imap_resource, _basic_webmail_get_server_string(), '*');
  if (empty($mailboxes[4])) {
    _basic_webmail_subscribe_to_mailboxes($user->uid);
  }

  // Remove the existing saved messages.
  db_delete('basic_webmail_messages')
    ->condition('user_id', $user->uid)
    ->condition('folder', urlencode($folder))      
    ->execute();

  if (imap_num_msg($imap_resource) > 0) {
    // Retrieve and display the mail in the current folder.
    $thread = imap_thread($imap_resource);
    if ($thread === FALSE) {
      // Report the error.
      $message = t('There was an error getting the list of messages.');
      _basic_webmail_report_error($message, array(), WATCHDOG_ERROR, TRUE);
      return;
    }
    else {
      $root = 0; 
      $rootValues = array(); 
      $threads = array();
      // source: http://php.net/manual/en/function.imap-thread.php (comments)
      // First we find the root (or parent) value for each email in the thread 
      // we ignore emails that have no root value except those that are infact 
      // the root of a thread 
      // we want to gather the message IDs in a way where we can get the details of 
      // all emails on one call rather than individual calls ( for performance ) 
      foreach ($thread as $i => $messageId) {
        //get sequence and type 
        list($sequence, $type) = explode('.', $i);

        //if type is not num or messageId is 0 or (start of a new thread and no next) or is already set 
        if ($type != 'num' || $messageId == 0 || isset($rootValues[$messageId])) {
          //ignore it 
          continue;
        }
        //if this is the start of a new thread 
        if ($root == 0) {
          //set root 
          $root = $messageId;
        }
        //at this point this will be part of a thread 
        //let's remember the root for this email 
        $rootValues[$messageId] = $root;

        //if there is no next 
        if ($thread[$sequence . '.next'] == 0) {
          //reset root 
          $root = 0;
        }
      }
      //now get all the emails details in rootValues in one call 
      //because one call for 1000 rows to a server is better 
      //than calling the server 1000 times  
      $emails = imap_fetch_overview($imap_resource, implode(',', array_keys($rootValues))); 

      //foreach email 
      foreach ($emails as $email) {
        //get root 
        $root = $rootValues[$email->msgno];

        //add to threads 
        $threads[$root] = $email;
      }
      
      // Save the message information in a temporary table.
      foreach ($threads as $key => $message) {
        // Make sure the message is not marked as deleted.
        if ($message->deleted == 0) {
          // Set the read status.
          if ($message->seen == 0) {
            $message_unread = 1;
          } else {
            $message_unread = 0;
          }
  
          // Process the message's subject for storage in the database.
          if (!empty($message->subject)) {
            $message_subject = _basic_webmail_process_subject($message->subject, $message_unread, $key, $folder);
          } else {
            $message_subject = _basic_webmail_process_subject('(No subject specified)', $message_unread, $key, $folder);
          }
  
          // Process the message sender for storage in the database.
          if (empty($message->from)) {
            $from_address = '';
          } else {
            $from_address = $message->from;
          }

          // Set the message's date.
          if (empty($message->date)) {
            $message_date = 0;
          } else {
            $message_date = strtotime($message->date);
          }

          // imap_uid($imap_resource, $message_number)
          $id = db_insert('basic_webmail_messages')
                  ->fields(array(
                      'user_id' => $user->uid,
                      'message_number' => $key,
                      'message_subject' => $message_subject,
                      'from_address' => $from_address,
                      'message_date' => $message_date,
                      'message_unread' => $message_unread,
                      'folder' => urlencode($folder),
                  ))
                  ->execute();
        }
      }
    }
    variable_set('basic_webmail_block_content_last_pull_timestamp', time());
    error_reporting(0);
  }

  // Clean up.
  if (!imap_close($imap_resource)) {
    $message = 'There was an error closing the IMAP stream.';
    _basic_webmail_report_error($message, array(), WATCHDOG_WARNING);
  }
}


/**
 * Retrieves the list of messages.
 *
 * @param array $header
 *   The original header that was created in
 *   _basic_webmail_create_message_list_header().
 * @param integer $user_id
 *   The ID for the user to gather messages for.
 * @param folder
 *   The folder mailbox
 * @param limit
 *   The limit of messages, if nothing is passed it uses the variable value.
 *
 * @return array
 *   The list of messages.
 */
function _basic_webmail_get_message_list_data($header, $user_id, $folder, $limit = null, $from = '') {
  // Initialize variables.
  $message_list = array();
  $icon_path = drupal_get_path('module', 'basic_webmail') . '/images/';

  // Specifiy the number of messages to display on a page.
  $row_limit = $limit;
  if($row_limit == null) {
    $row_limit = variable_get('basic_webmail_messages_per_page', 25);
  }
  
  // Construct and execute the database query.
  $result = db_select('basic_webmail_messages', 'm')
    ->fields('m', array(
      'message_number',
      'message_unread',
      'message_subject',
      'from_address',
      'message_date',
    ))
    ->condition('user_id', $user_id, '=')
    ->condition('folder', urlencode($folder), '=')
    ->condition('from_address', '%'. $from .'%', 'LIKE')
    ->extend('PagerDefault')
    ->limit($row_limit)
    ->extend('TableSort')
    ->orderByHeader($header)
    ->execute();

  // Iterate through the messages.
  foreach ($result as $message) {
    // Initialize variables.
    $message_unread = '';
    $message_date = '(Invalid date.)';

    // If the message is unread, show the icon.
    if (!empty($message->message_unread)) {
      $message_unread = theme_image(array(
        'path' => $icon_path . 'mini-mail.png',
        'alt' => 'Generic icon indicating message is unread.',
        'title' => t('You have not read this message.'),
        'attributes' => array(),
      ));
    }
    elseif (empty($message->message_unread)) {
      $message_unread = theme_image(array(
        'path' => $icon_path . 'mini-mail-open.png',
        'alt' => 'Generic icon indicating message is read.',
        'title' => t('You have read this message.'),
        'attributes' => array(),
      ));
    }

    // The date the message was sent.
    if ($message->message_date != 0) {
      $format_option = variable_get('basic_webmail_format_option', 'small');

      if ($format_option == 'custom') {
        $custom_format = variable_get('basic_webmail_custom_format', 'D, M j, Y - g:i:s a');
        $message->message_date = format_date($message->message_date, $format_option, $custom_format);
      }
      else {
        $message->message_date = format_date($message->message_date, $format_option);
      }

      $message_date = $message->message_date;
    }

    $message_list[$message->message_number] = array(
      'message_unread' => $message_unread,
      'message_subject' => $message->message_subject,
      'from_address' => $message->from_address,
      'message_date' => $message_date,
    );
  }

  return $message_list;
}


/**
 * Retrieves the page the user was viewing when they chose their action.
 *
 * @return array
 *   The previous page the user was viewing when they took the action they
 *   chose. Basically, this uses the parameters of the drupal_goto function. For
 *   a list of possible options, see the url function.
 */
function _basic_webmail_get_return_destination() {
  $orig_path = drupal_get_destination();
  $orig_length = drupal_strlen($orig_path['destination']);
  $orig_start = strpos($orig_path['destination'], '=');
  $new_path = drupal_substr($orig_path['destination'], $orig_start, $orig_length);

  // Restore the converted characters.
  $new_path = str_replace('%25', '%', $new_path);
  $new_path = str_replace('%2F', '/', $new_path);
  $new_path = str_replace('%3F', '?', $new_path);
  $new_path = str_replace('%3D', '=', $new_path);
  $new_path = str_replace('%26', '&', $new_path);

  // Setup the return parameters.
  $query_point = strpos($new_path, '?');
  if ($query_point === FALSE) {
    $dest_path = $new_path;
    $dest_query = NULL;
  }
  else {
    $dest_path = drupal_substr($new_path, 0, strpos($new_path, '?'));
    $dest_query = drupal_substr($new_path, strpos($new_path, '?') + 1, drupal_strlen($new_path));
  }

  // Send the user back to the page they were looking at.
  return array($dest_path, array('query' => $dest_query));
}


/**
 * Deletes a group of messages.
 *
 * @param array $form_state
 *   The form values of the passed form.
 * @param array $messages
 *   The messasges to delete.
 * @param stream $imap_resource
 *   An IMAP stream, which is a connection to the mailbox on the server, and is
 *   created by calling _basic_webmail_connect_to_server.
 */
function _basic_webmail_mail_list_form_submit_delete($form_state, $messages, $imap_resource) {
  $error = 0;
  $message_count = 0;

  foreach ($messages as $message_number) {
    if ($form_state['values']['folder'] != 'INBOX.Trash') {
      if (!imap_mail_move($imap_resource, $message_number, 'INBOX.Trash')) {
        include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
        // Report the error.
        $message = 'There was an error deleting message #@message_number.';
        $message_parameters = array('@message_number' => $message_number);
        _basic_webmail_report_error($message, $message_parameters, WATCHDOG_ERROR, TRUE);
        $error++;
      }
      else {
        $message_count++;
      }
    }
    else {
      if (!imap_delete($imap_resource, $message_number)) {
        include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
        // Report the error.
        $message = 'There was an error deleting message #@message_number.';
        $message_parameters = array('@message_number' => $message_number);
        _basic_webmail_report_error($message, $message_parameters, WATCHDOG_ERROR, TRUE);
        $error++;
      }
      else {
        $message_count++;
      }
    }
  }

  if (!imap_expunge($imap_resource)) {
    include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
    // Report the error.
    $message = 'There was an error expunging the mailboxes.';
    _basic_webmail_report_error($message, array(), WATCHDOG_WARNING);
    $error++;
  }

  if (empty($error)) {
    drupal_set_message(t('@message_count messages were successfully deleted.', array('@message_count' => $message_count)));
  }
}


/**
 * Marks a group of messages as read.
 *
 * @param array $form_state
 *   The form values of the passed form.
 * @param array $messages
 *   The messasges to delete.
 * @param stream $imap_resource
 *   An IMAP stream, which is a connection to the mailbox on the server, and is
 *   created by calling _basic_webmail_connect_to_server.
 */
function _basic_webmail_mail_list_form_submit_mark_read($form_state, $messages, $imap_resource) {
  $error = 0;
  $message_count = 0;

  foreach ($messages as $message_number) {
    if (!imap_setflag_full($imap_resource, $message_number, '\\Seen')) {
      include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
      // Report the error.
      $message = 'There was an error setting the flag of message #@message_number.';
      $message_parameters = array('@message_number' => $message_number);
      _basic_webmail_report_error($message, $message_parameters, WATCHDOG_ERROR, TRUE);
      $error++;
    }
    else {
      $message_count++;
    }
  }

  if (empty($error)) {
    drupal_set_message(t('@message_count messages were successfully marked as read.', array('@message_count' => $message_count)));
  }
}


/**
 * Marks a group of messages as unread.
 *
 * @param array $form_state
 *   The form values of the passed form.
 * @param array $messages
 *   The messasges to delete.
 * @param stream $imap_resource
 *   An IMAP stream, which is a connection to the mailbox on the server, and is
 *   created by calling _basic_webmail_connect_to_server.
 */
function _basic_webmail_mail_list_form_submit_mark_unread($form_state, $messages, $imap_resource) {
  $error = 0;
  $message_count = 0;

  foreach ($messages as $message_number) {
    if (!imap_clearflag_full($imap_resource, $message_number, '\\Seen')) {
      include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
      // Report the error.
      $message = 'There was an error clearing the flag of message #@message_number.';
      $message_parameters = array('@message_number' => $message_number);
      _basic_webmail_report_error($message, $message_parameters, WATCHDOG_ERROR, TRUE);
      $error++;
    }
    else {
      $message_count++;
    }
  }

  if (empty($error)) {
    drupal_set_message(t('@message_count messages were successfully marked as unread.', array('@message_count' => $message_count)));
  }
}


/**
 * Copies a group of messages to another folder.
 *
 * @param array $form_state
 *   The form values of the passed form.
 * @param array $messages
 *   The messasges to delete.
 * @param stream $imap_resource
 *   An IMAP stream, which is a connection to the mailbox on the server, and is
 *   created by calling _basic_webmail_connect_to_server.
 */
function _basic_webmail_mail_list_form_submit_copy_to($form_state, $messages, $imap_resource) {
  $error = 0;
  $message_count = 0;

  foreach ($messages as $message_number) {
    if (!imap_mail_copy($imap_resource, $message_number, $form_state['values']['folder_name'])) {
      include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
      // Report the error.
      $message = 'There was an error copying message #@message_number.';
      $message_parameters = array('@message_number' => $message_number);
      _basic_webmail_report_error($message, $message_parameters, WATCHDOG_ERROR, TRUE);
      $error++;
    }
    else {
      $message_count++;
    }
  }

  if (!imap_expunge($imap_resource)) {
    include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
    // Report the error.
    $message = 'There was an error expunging the mailboxes.';
    _basic_webmail_report_error($message, array(), WATCHDOG_WARNING);
    $error++;
  }

  if (empty($error)) {
    drupal_set_message(t('@message_count messages were successfully copied to @folder_name.', array('@message_count' => $message_count, '@folder_name' => $form_state['values']['folder_name'])));
  }
}


/**
 * Moves a group of messages to another folder.
 *
 * @param array $form_state
 *   The form values of the passed form.
 * @param array $messages
 *   The messasges to delete.
 * @param stream $imap_resource
 *   An IMAP stream, which is a connection to the mailbox on the server, and is
 *   created by calling _basic_webmail_connect_to_server.
 */
function _basic_webmail_mail_list_form_submit_move_to($form_state, $messages, $imap_resource) {
  $error = 0;
  $message_count = 0;

  foreach ($messages as $message_number) {
    if (!imap_mail_move($imap_resource, $message_number, $form_state['values']['folder_name'])) {
      include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
      // Report the error.
      $message = 'There was an error moving message #@message_number.';
      $message_parameters = array('@message_number' => $message_number);
      _basic_webmail_report_error($message, $message_parameters, WATCHDOG_ERROR, TRUE);
      $error++;
    }
    else {
      $message_count++;
    }
  }

  if (!imap_expunge($imap_resource)) {
    include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
    // Report the error.
    $message = 'There was an error expunging the mailboxes.';
    _basic_webmail_report_error($message, array(), WATCHDOG_WARNING);
    $error++;
  }

  if (empty($error)) {
    drupal_set_message(t('@message_count messages were successfully moved to @folder_name.', array('@message_count' => $message_count, '@folder_name' => $form_state['values']['folder_name'])));
  }
}


/**
 * Checks if the standard mailboxes exist and creates them, if necessary.
 *
 * This function checks to see if the standard mailboxes exit or not. If they
 * do not exist, this function will also create them. This fucntion will then
 * subscribe to the mailboxes.
 *
 * @param integer $user_id
 *   The user ID of the mailbox to subscribe to.
 */
function _basic_webmail_subscribe_to_mailboxes($user_id) {
  // Connect to the server and retrieve a connection to the mailbox.
  include_once drupal_get_path('module', 'basic_webmail') . '/basic_webmail.common.inc';
  $imap_resource = _basic_webmail_connect_to_server($user_id);
  if (empty($imap_resource)) {
    $message = 'There was an error connecting to the mail server. Contact the system administrator and/or check the logs for more information.';
    _basic_webmail_report_error($message, array(), WATCHDOG_ERROR, TRUE);
    return;
  }

  $mailboxes = array(
    'INBOX',
    'INBOX.Drafts',
    'INBOX.Junk',
    'INBOX.Sent',
    'INBOX.Trash',
  );
  foreach ($mailboxes as $mailbox) {
    if (!imap_status($imap_resource, _basic_webmail_get_server_string() . $mailbox, SA_ALL)) {
      if (!imap_createmailbox($imap_resource, _basic_webmail_get_server_string() . $mailbox)) {
        $message = 'There was an error creating to the !mailbox mailbox.';
        $message_parameters = array('!mailbox' => $mailbox);
        _basic_webmail_report_error($message, $message_parameters, WATCHDOG_WARNING);
      }
    }

    if (!imap_subscribe($imap_resource, _basic_webmail_get_server_string() . $mailbox)) {
      $message = 'There was an error subscribing to the !mailbox mailbox.';
      $message_parameters = array('!mailbox' => $mailbox);
      _basic_webmail_report_error($message, $message_parameters, WATCHDOG_WARNING);
    }
  }

  // Clean up.
  if (!imap_close($imap_resource)) {
    $message = 'There was an error closing the IMAP stream.';
    _basic_webmail_report_error($message, array(), WATCHDOG_WARNING);
  }
}


/**
 * Processes a messasge's subject for storage in the database.
 *
 * @param string $subject
 *   The subject of the message.
 * @param integer $new_or_unread
 *   Indicates whether the message is new or unread or neither. If the message
 *   is new or unread, this is TRUE.
 * @param integer $message_number
 *   The number of the message in the mailbox.
 * @param string $folder
 *   The folder the message is in.
 *
 * @return string
 *   The subject of the message processed as a link for display and for storage
 *   in the database.
 */
function _basic_webmail_process_subject($subject, $new_or_unread, $message_number, $folder) {
  $number_characters = variable_get('basic_webmail_subject_characters', 40);

  if ($subject != '(No subject specified)') {
    $subject = _basic_webmail_prepare_email_string($subject);

    // If the link to the subject is longer than the database field will allow,
    // shorten the displayed subject so it fits, accounting for the ellipsis.
    $link_len = drupal_strlen($folder) + drupal_strlen($message_number) + 55;

    // Check to see if the admin specified a maximum character length for the
    // displayed subject.
    if (!empty($number_characters)) {
      // Make sure that length is not longer than the database field will allow.
      if (255 - $link_len < $number_characters) {
        $subject = truncate_utf8($subject, 255 - $link_len, TRUE, TRUE);
      }
      else {
        $subject = truncate_utf8($subject, $number_characters, TRUE, TRUE);
      }
    }
    elseif (drupal_strlen($subject) + $link_len > 255) {
      $subject = truncate_utf8($subject, 255 - $link_len, TRUE, TRUE);
    }
  }

  // Create a link to the message.
  $message_subject_link = l($subject, 'basic_webmail/view/' . $folder . '/' . $message_number);

  // Set the message's read status.
  if (!empty($new_or_unread)) {
    $message_subject = '<strong>' . $message_subject_link . '</strong>';
  }
  else {
    $message_subject = $message_subject_link;
  }

  return $message_subject;
}
