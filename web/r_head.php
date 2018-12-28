<?php

if (isset($_GET['id_m_head'])) {
    $id_m_head = $_GET['id_m_head'];
} else {
    $id_m_head = 0;
}

//print"Paremeter:\n";
//var_dump($id_m_head);

try {
	$str_conn = "mysql:host=bergulix.dyndns.org:3306;dbname=bx_bow_01";
	$dbh = new PDO($str_conn, 'msql1', 'Qasde321' , array(PDO::ATTR_PERSISTENT => true));
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
	
	$sql = "select distinct
		id_m_head,
		mdate,
		bow.bow_description,
		person.person_name,
		gps_lat,
		gps_lon,
		group_concat(imu_serial separator ',') as ImuS
		from m_head 
		left join m_imu on fk_id_m_head=id_m_head
		left join bow on bow.id_bow=m_head.fk_bow_id
		left join person on person.id_person=m_head.fk_person_id
		group by 1
		having id_m_head=".$id_m_head." or ".$id_m_head."=0";

    foreach ($dbh->query($sql) as $row) {
        $data[] = array(
        	'id_m_head'=>$row['id_m_head'],
        	'mdate'=>$row['mdate']
        );
        //print $row['id_m_head'] . " ";
    }

	print "<br><br> Dump Data <br><br>";
	var_dump($data);

	print "<br><br> XML:  <br><br>";

	$x = new SimpleXMLElement('<root/>');
	$x = $x->addChild("Hello");
	print $x;

	print "<br><br> XML 1:  <br><br>";

	$xml = xml_encode($data);

    // echo "$xml\n";
    $dom = new DOMDocument;
    $dom->preserveWhiteSpace = FALSE;
    $dom->loadXML($xml);
    $dom->formatOutput = TRUE;
    echo $dom->saveXml();

	print "<br><br> XML 2:  <br><br>";

	$xml = arrayToXML($data);
	print $xml;





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


// ---------------------------------------------------------------

function array_to_xml(array $arr, SimpleXMLElement $xml) {
        foreach ($arr as $k => $v) {
            $attrArr = array();
            $kArray = explode(' ',$k);
            $tag = array_shift($kArray);
            if (count($kArray) > 0) {
                foreach($kArray as $attrValue) {
                    $attrArr[] = explode('=',$attrValue);                   
                }
            }
            if (is_array($v)) {
                if (is_numeric($k)) {
                    array_to_xml($v, $xml);
                } else {
                    $child = $xml->addChild($tag);
                    if (isset($attrArr)) {
                        foreach($attrArr as $attrArrV) {
                            $child->addAttribute($attrArrV[0],$attrArrV[1]);
                        }
                    }                   
                    array_to_xml($v, $child);
                }
            } else {
                $child = $xml->addChild($tag, $v);
                if (isset($attrArr)) {
                    foreach($attrArr as $attrArrV) {
                        $child->addAttribute($attrArrV[0],$attrArrV[1]);
                    }
                }
            }               
        }
        return $xml;
    }

// ---------------------------------------------------------------
function generateXML($tag_in,$value_in="",$attribute_in=""){
    $return = "";
    $attributes_out = "";
    if (is_array($attribute_in)){
        if (count($attribute_in) != 0){
            foreach($attribute_in as $k=>$v):
                $attributes_out .= " ".$k."=\"".$v."\"";
            endforeach;
        }
    }
    return "<".$tag_in."".$attributes_out.((trim($value_in) == "") ? "/>" : ">".$value_in."</".$tag_in.">" );
}

function arrayToXML($array_in){
    $return = "";
    $attributes = array();
    foreach($array_in as $k=>$v):
        if ($k[0] == "@"){
            // attribute...
            $attributes[str_replace("@","",$k)] = $v;
        } else {
            if (is_array($v)){
                $return .= generateXML($k,arrayToXML($v),$attributes);
                $attributes = array();
            } else if (is_bool($v)) {
                $return .= generateXML($k,(($v==true)? "true" : "false"),$attributes);
                $attributes = array();
            } else {
                $return .= generateXML($k,$v,$attributes);
                $attributes = array();
            }
        }
    endforeach;
    return $return;
} 

//---------------------------

function addElements(&$xml,$array)
{
$params=array();
foreach($array as $k=>$v)
{
    if(is_array($v))
        addElements($xml->addChild($k), $v);
    else $xml->addAttribute($k,$v);
}

}
function xml_encode($array)
{
if(!is_array($array))
    trigger_error("Type missmatch xml_encode",E_USER_ERROR);
//$xml=new SimpleXMLElement('<?xml version=\'1.0\' encoding=\'utf-8\'?><'.key($array).'/>');
$xml=new SimpleXMLElement('<root/>');
addElements($xml,$array[key($array)]);
return $xml->asXML();
} 
?>