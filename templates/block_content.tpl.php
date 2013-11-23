<?php
/* arrays structure received
 * 
 * filtering = 0 or 1
 * from = string
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
<?php
  if (!isset($is_filtering)) $is_filtering = 0;
  if (!isset($from)) $from = '';
  if (!isset($inbox)) $inbox = array();
  if (!isset($sent)) $sent = array();
?>
<style>
  #block-basic-webmail-basic-webmail-block li a{
    float:none;
    display:inline;
  }
</style>  
<?php if ($is_filtering == 1): ?>
<p><b>Filter:</b> <?php print $from; ?></p>
<?php else: ?>
<p><b>Filter:</b> All</p>
<?php endif; ?>
<p><h3>Inbox</h3></p>
<ul>
  <?php if (count($inbox) > 0): ?>
    <?php foreach($inbox as $msg): ?>
    <li><b><?php print $msg['message_date'];?></b> | <span alt='<?php print $msg['from_address'];?>'><?php print $msg['message_subject'];?></span></li>
    <?php endforeach; ?>
  <?php else: ?>
    <li><i>&lt empty &gt;</i></li>
  <?php endif; ?>
</ul>

<p><h3>Sent</h3></p>
<ul>
  <?php if (count($sent) > 0): ?>
    <?php foreach($sent as $msg): ?>
    <li><b><?php print $msg['message_date'];?></b> | <span alt='<?php print $msg['from_address'];?>'><?php print $msg['message_subject'];?></span></li>
    <?php endforeach; ?>
  <?php else: ?>
    <li><i>&lt empty &gt;</i></li>
  <?php endif; ?>
</ul>