<?php
/* spt/list.php - list found prime tuples from database */
require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");

function convert_tuple($ofs,$k) {
	$d= explode(' ',$ofs);
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
	return $astr;
}

function show_by_batch_group_k($f_batch) {
  $fmt=get_int('fmt',true);
  header("Content-type: text/plain");
  $db = BoincDb::get();
  $set = $db->do_query("select distinct(start), id, k, ofs from spt where kind in ('spt','stpt') and batch=$f_batch order by k, start");
  $prevk=0;
  echo "# Copyright Tomas Brada, ask on forum about reuse or citation.\n";
	echo "# where batch = $f_batch\n";
	$lastid=-1; $rcount=0;
  while($row = $set->fetch_object('stdClass')) {
		if($fmt==3) {
			$astr = "k={$row->k} {$row->ofs}";
		} else {
			$astr=convert_tuple($row->ofs,$row->k);
		}
		if($row->k!=$prevk) {
			echo "k=$row->k\n";
			$prevk=$row->k;
		}
    echo "{$row->start}: $astr\n";
    if($row->id>$lastid) $lastid=$row->id; $rcount++;
  }
  echo "# last = $lastid\n";
  $set->free();
}

function show_by_k() {
	$k= get_int('k');
	$minid = get_int('i',true);
  header("Content-type: text/plain");
  $db = BoincDb::get();
  $minidclausule = "";
  if($minid!==null)
		$minidclausule = "and id>$minid";
  $set = $db->do_query("select distinct(start), id, k, ofs from spt where kind in ('spt','stpt') and k=$k $minidclausule order by start");
  $prevk=0;
  echo "# Copyright Tomas Brada, ask on forum about reuse or citation.\n";
	echo "# where k = $k\n";
  if($minid!==null)
		echo "# where id > $minid\n";
	$lastid=-1; $rcount=0;
  $prev=0;
  while($row = $set->fetch_object('stdClass')) {
    if($row->start==$prev)
      continue;
    $prev=$row->start;
		$astr=convert_tuple($row->ofs,$row->k);
    echo "{$row->start}: $astr\n";
    if($row->id>$lastid) $lastid=$row->id; $rcount++;
  }
  echo "# last = $lastid # count = $rcount\n";
  $set->free();
}

function main() {
  if(null!==$f_batch=get_int('batch',true)) {
		show_by_batch_group_k($f_batch);
	} else if (null!==get_int('k',true)) {
		show_by_k();
	}
	// else error todo
}

main();
