<?php

$hd = $_POST['m_id'];
$dt = $_POST['data'];

//$header = preg_replace( "/\r|\n/", "", $hd );
//$data = preg_replace( "/\r|\n/", "", $dt );
$header = $hd;
$data = $dt;

//print"Data:\n";
//var_dump($header);
//var_dump($data);

try {
	$str_conn = "mysql:host=bergulix.dyndns.org:3306;dbname=bx_bow_01";
	$dbh = new PDO($str_conn, 'msql1', 'Qasde321' , array(PDO::ATTR_PERSISTENT => true));
	$dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
	$dbh->beginTransaction();    	
} catch (Exception $e) {
	print_r (sprintf(
		'%s <br />
		CallStack: <br />
		%s',
		$e->getMessage(),
		$e->getTraceAsString()
	));			
	exit;   	
}    

try {
	
	$sql = "INSERT INTO m_data (time_raw, acc_x, acc_y, acc_z, fk_id_m_imu) VALUES (?,?,?,?,?)";
	
	$sorok = explode(";", $data);
	//var_dump($sorok);
	foreach ($sorok as $sorsazam => $sor_adatok) {
		$stmt = $dbh->prepare($sql);
		if ($sor_adatok > "") {
			$sor_tomb = explode(',', $sor_adatok);
			$sor_tomb[] = $header;
			$stmt->execute($sor_tomb);	
			$index = $dbh->lastInsertId();
			//var_dump($index);
		}
	}

	$dbh->commit();

	echo "OK";

} catch (Exception $e) {
	$dbh->rollBack();
	print_r (sprintf(
		'HIBA: <br /> 
		%s <br />
		CallStack: <br />
		%s',
		$e->getMessage(),
		$e->getTraceAsString()
	));
	var_dump($sor_tomb);
}
?>