<?php


function simple_xmlify($arr, SimpleXMLElement $root = null, $el = 'x') {
    // based on, among others http://stackoverflow.com/a/1397164/1037948

    if(!isset($root) || null == $root) $root = new SimpleXMLElement('<' . $el . '/>');

    if(is_array($arr)) {
        foreach($arr as $k => $v) {
            // special: attributes
            if(is_string($k) && $k[0] == '@') $root->addAttribute(substr($k, 1),$v);
            // normal: append
            else simple_xmlify($v, $root->addChild(
                    // fix 'invalid xml name' by prefixing numeric keys
                    is_numeric($k) ? 'n' . $k : $k)
                );
        }
    } else {
        $root[0] = $arr;
    }

    return $root;
}//--   fn  simple_xmlify

function get_formatted_xml(SimpleXMLElement $xml, $domver = null, $preserveWhitespace = true, $formatOutput = true) {
    // http://stackoverflow.com/questions/1191167/format-output-of-simplexml-asxml
    // create new wrapper, so we can get formatting options
    $dom = new DOMDocument($domver);
    $dom->preserveWhiteSpace = $preserveWhitespace;
    $dom->formatOutput = $formatOutput;
    // now import the xml (converted to dom format)
    /*
    $ix = dom_import_simplexml($xml);
    $ix = $dom->importNode($ix, true);
    $dom->appendChild($ix);
    */
    $dom->loadXML($xml->asXML());

    // print
    return $dom->saveXML();
}//--   fn  get_formatted_xml

// lazy declaration via "queryparam"
$args = 'hello=4&var[]=first&var[]=second&foo=1234&var[5]=fifth&var[sub][]=sub1&var[sub][]=sub2&var[sub][]=sub3&var[@name]=the-name&var[@attr2]=something-else&var[sub][@x]=4.356&var[sub][@y]=-9.2252';
$q = array();
parse_str($args, $q);

$xml = simple_xmlify($q); // dump $xml, or...
echo $xml->asXML;

//$result = get_formatted_xml($xml); // see below

//print $result
?>