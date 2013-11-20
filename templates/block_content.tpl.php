<?php
/* arrays structure received
 * 
 * received = array(
 *   [0] = array(
 *     'header' = string,
 *     'date' = string, 
 *     'from' = string,
 *   )
 *   [1] = array(
 *     'header' = string,
 *     'date' = string, 
 *     'from' = string,
 *   )
 *   ...
 * );
 *  
 * sent = array(
 *   [0] = array(
 *     'header' = string,
 *     'date' = string, 
 *     'from' = string,
 *   )
 *   [1] = array(
 *     'header' = string,
 *     'date' = string, 
 *     'from' = string,
 *   )
 *   ...
 * ); 
 */

?>

<p><b>Inbox</b></p>
<ul>
  <?php foreach($received as $msg): ?>
  <li><b>Date:</b><?php print $msg['date'];?>  <b>Header:</b><?php print $msg['header'];?>  <b>From:</b><?php print $msg['from'];?></li>
  <?php endforeach; ?>
</ul>

<p><b>Sent</b></p>
<ul>
  <?php foreach($sent as $msg): ?>
  <li><b>Date:</b><?php print $msg['date'];?>  <b>Header:</b><?php print $msg['header'];?>  <b>From:</b><?php print $msg['from'];?></li>
  <?php endforeach; ?>
</ul>