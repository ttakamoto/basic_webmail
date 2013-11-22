<?php
/* arrays structure received
 * 
 * inbox = array(
 *   [0] = array(
 *     'message_subject' = string,
 *     'message_date' = string, 
 *     'from_address' = string,
 *   )
 *   [1] = array(
 *     'message_subject' = string,
 *     'message_date' = string, 
 *     'from_address' = string,
 *   )
 *   ...
 * );
 *  
 * sent = array(
 *   [0] = array(
 *     'message_subject' = string,
 *     'message_message_date' = string, 
 *     'from_address' = string,
 *   )
 *   [1] = array(
 *     'message_subject' = string,
 *     'message_date' = string, 
 *     'from_address' = string,
 *   )
 *   ...
 * ); 
 */

?>

<p><b>Inbox</b></p>
<ul>
  <?php foreach($inbox as $msg): ?>
  <li><b>Date:</b><?php print $msg['message_date'];?>  <b>Header:</b><?php print $msg['message_subject'];?>  <b>From:</b><?php print $msg['from_address'];?></li>
  <?php endforeach; ?>
</ul>

<p><b>Sent</b></p>
<ul>
  <?php foreach($sent as $msg): ?>
  <li><b>Date:</b><?php print $msg['message_date'];?>  <b>Header:</b><?php print $msg['message_subject'];?>  <b>From:</b><?php print $msg['from_address'];?></li>
  <?php endforeach; ?>
</ul>