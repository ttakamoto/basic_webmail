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
<style>
  #block-basic-webmail-basic-webmail-block li a{
    float:none;
    display:inline;
  }
</style>  
<p><h3>Inbox</h3></p>
<ul>
  <?php foreach($inbox as $msg): ?>
  <li><b><?php print $msg['message_date'];?></b> | <span alt='<?php print $msg['from_address'];?>'><?php print $msg['message_subject'];?></span></li>
  <?php endforeach; ?>
</ul>

<p><h3>Sent</h3></p>
<ul>
  <?php foreach($sent as $msg): ?>
  <li><b><?php print $msg['message_date'];?></b> | <span alt='<?php print $msg['from_address'];?>'><?php print $msg['message_subject'];?></span></li>
  <?php endforeach; ?>
</ul>