<?php
/* spt/list.php - list found prime tuples from database */
require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");

function main() {
  $f_batch=get_int('batch');
  //$fmt=get_int('fmt');
  header("Content-type: text/plain");
  $db = BoincDb::get();
  $set = $db->do_query("select distinct(start), k, ofs from spt where batch=$f_batch order by k, start");
  $prevk=0;
  echo "# Copyright Tomas Brada, ask on forum about reuse or citation.\n";
  while($row = $set->fetch_object('stdClass')) {
    $d= explode(' ',$row->ofs);
    $k=$row->k;
    $d=array_pad($d,$k-1,0);
    //mirror d
    for($i=0,$j=$k-2;$i<$j;$i++,$j--) {
      $d[$j]=$d[$i];
    }
    $a=array(0);
    for($i=1;$i<$k;$i++) {
      $a[$i] = $a[$i-1] + $d[$i-1];
    }
    $astr=implode(' ',$a);
    $dstr=implode(' ',$d);
    if($k!=$prevk) {
      echo "k=$k\n";
      $prevk=$k;
    }
    echo "{$row->start}: $astr\n";
  }
  $set->free();
}

main();
