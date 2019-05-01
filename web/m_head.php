<?php

$dt = $_POST['data'];

//$header = preg_replace( "/\r|\n/", "", $hd );
//$data = preg_replace( "/\r|\n/", "", $dt );

$data = $dt;

//print"Data:\n";
//var_dump($data);

try {
	$str_conn = "mysql:host=bergulix.dyndns.org:3306;dbname=bx_bow_01";
	$dbh = new PDO($str_conn, 'msql1', 'Bergulix!1234' , array(PDO::ATTR_PERSISTENT => true));
	$dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
	$dbh->beginTransaction();    	
} catch (Exception $e) {
	print_r (sprintf(
		'Database connection error %s <br />
		CallStack: <br />
		%s',
		$e->getMessage(),
		$e->getTraceAsString()
	));			
	exit;   	
}    

try {
	
	$sql = "INSERT INTO m_head (mdate, fk_bow_id, fk_person_id, gps_lat, gps_lon) VALUES (?,?,?,?,?)";
	
	$stmt = $dbh->prepare($sql);
	if ($data > "") {
		$sor_tomb = explode(',', $data);
		$stmt->execute($sor_tomb);	
		$index = $dbh->lastInsertId();
	}

	$dbh->commit();

	//print "OK ",$index;
	print_r (sprintf(
		'OK %s',$index
	));	


} catch (Exception $e) {
	$dbh->rollBack();
	print_r (sprintf(
		'Data insert error: <br /> 
		%s <br />
		CallStack: <br />
		%s',
		$e->getMessage(),
		$e->getTraceAsString()
	));
	var_dump($sor_tomb);
}
?>