<?php
// For displays errors codes with exit
ini_set('display_errors', 'stderr');

$opcodeCount = 1;
$headerFound = false;
$labelStack = array();
$labelsCount = 0;

// Adding program root element
$program = new SimpleXMLElement('<?xml version="1.0" encoding="UTF-8"?>'.'<program> </program>');
$program->addAttribute('language', 'IPPcode21');

// Function takes an argument and returns type and value in array.
// Alternatively, it checks correctness of the names (variables, messages, literals).
function typeCheck($argument)
{
    $returnArray = [];
    if(strpos($argument, '@') !== false){
        $arr = explode('@', $argument);
        if ($arr[0] === "TF" || $arr[0] === "GF" || $arr[0] === "LF") {
            array_push($returnArray, "var");
            if(preg_match("/^[A-za-z_\$\&\%\*\!\?\-][a-zA-Z_0-9_\$\&\%\*\!\?\-]*$/", $arr[1])){
                array_push($returnArray, $argument);
                return($returnArray);
            }
        }
        elseif($arr[0] === "int") {
            if(!preg_match("/^[\+\-0-9][0-9]*$/", $arr[1])) {
                exit(23);
            }
            array_push($returnArray, "int");
            array_push($returnArray, $arr[1]);
            return($returnArray);
        }
        elseif($arr[0] === "string") {
            if(!preg_match("/^([^\\\\]|\\\\\d\d)*$/", $arr[1])) {  
                exit(23);
            }
            array_push($returnArray, "string");
            array_push($returnArray, $arr[1]);
            return($returnArray);
        }
        elseif($arr[0] === "bool"){
            if(strtolower($arr[1]) ==="true" || strtolower($arr[1])=== "false") {
                array_push($returnArray, "bool");
                array_push($returnArray, $arr[1]);
                return($returnArray);
            }
            else {
                exit(23);
            }
        }
        elseif($arr[0] === "nil"){
            array_push($returnArray, "nil");
            if($arr[1] ==="nil") {
                array_push($returnArray, $arr[1]);
            }
            else {
                exit(23);
            }
            return($returnArray);
        }
        else {
            exit(23);
        }
    }
    else {
        if ($argument=="int" || $argument=="string" || $argument=="bool" || $argument=="nil") {
            array_push($returnArray, "type");
            array_push($returnArray, $argument);
            return $returnArray;
        }
        if(preg_match("/^[A-za-z_\$\&\%\*\!\?\-][a-zA-Z_0-9_\$\&\%\*\!\?]*$/", $argument)){
            array_push($returnArray, "label");
            array_push($returnArray, htmlspecialchars($argument));
            return $returnArray;
        }
        else {
            exit(23);
        }
    }
}

// Function checks types of each instruction argument.
function checkTypesOfArgument($instValue): void
{
    $instValue[0] = strtoupper($instValue[0]);

    switch($instValue[0]){
    case "MOVE":
    case "TYPE":
    case "NOT":
    case "INT2CHAR":

        $typeValueArray = typeCheck($instValue[1]);
        if($typeValueArray[0]!== "var") {
            exit(23);
        }
        $arrToXML = array();
        array_push($arrToXML, $instValue[0]);
        array_push($arrToXML, $typeValueArray);
        $arraySecond = typeCheck($instValue[2]);
        if($arraySecond[0] == "label" || $arraySecond[0] == "type") {
            exit(23);
        }
        array_push($arrToXML, $arraySecond);
        addToXML($arrToXML);
        break;
    
    case "CREATEFRAME":
    case "PUSHFRAME":
    case "POPFRAME":
    case "BREAK":
    case "RETURN":

        addToXML($instValue);
        break;
    
    case "DEFVAR":
    case "POPS":

        $array = typeCheck($instValue[1]);
        if($array[0]!== "var") {
            exit(23);
        }
        $arrToXML = array();
        array_push($arrToXML, $instValue[0]);
        array_push($arrToXML, $array);
        addToXML($arrToXML);
        break;

    case "CALL":
    case "LABEL":
    case "JUMP":

        global $labelStack;
        $array = typeCheck($instValue[1]);
        if($array[0]== "label" || $array[0]== "type"){
            $array[0] = "label";
        }
        else {
            exit(23);
        }
        if(!in_array($array[1], $labelStack)) {
            array_push($labelStack, htmlspecialchars($array[1]));
            global $labelsCount;
            $labelsCount++;
        }
        else {
            exit(23);
        }
        $arrToXML = array();
        array_push($arrToXML, $instValue[0]);
        array_push($arrToXML, $array);
        addToXML($arrToXML);
        break;

    case "PUSHS":
    case "DPRINT":
    case "WRITE":
    case "EXIT":

        $arrToXML = array();
        $typeValueArray = typeCheck($instValue[1]);
        array_push($arrToXML, $instValue[0]);
        array_push($arrToXML, $typeValueArray);
        if($typeValueArray[0] == "label" || $typeValueArray[0] == "type") {
            exit(23);
        }
        addToXML($arrToXML);
        break;

    case "ADD":
    case "SUB":
    case "MUL":
    case "IDIV":
    case "LT":
    case "GT":
    case "EQ":
    case "AND":
    case "OR":
    case "STRI2INT":
    case "GETCHAR":
    case "CONCAT":
    case "SETCHAR":

        $arrToXML = array();
        $typeValueArray = typeCheck($instValue[1]);
        if($typeValueArray[0]!=="var") {
            exit(23);
        }
        $arraySecond = typeCheck($instValue[2]);
        if($arraySecond[0] =="label" || $arraySecond[0] =="type") {
            exit(23);
        }
        $arrayThird = typeCheck($instValue[3]);
        if($arrayThird[0] =="label" || $arrayThird[0] =="type") {
            exit(23);
        }
        array_push($arrToXML, $instValue[0]);
        array_push($arrToXML, $typeValueArray);
        array_push($arrToXML, $arraySecond);
        array_push($arrToXML, $arrayThird);

        addToXML($arrToXML);
        break;


    case "READ":

        $arrToXML = array();
        $typeValueArray = typeCheck($instValue[1]);
        if($typeValueArray[0]!=="var") {
            exit(23);
        }
        $arraySecond = typeCheck($instValue[2]);
        if($arraySecond[0] == "type") {
            if ($arraySecond[1] != "int" && $arraySecond[1] != "string" && $arraySecond[1] != "bool") {
                exit(23);
            }
        }
        else {
            exit(23);
        }
        array_push($arrToXML, $instValue[0]);
        array_push($arrToXML, $typeValueArray);
        array_push($arrToXML, $arraySecond);
        addToXML($arrToXML);
        break;

    case "STRLEN":

        $arrToXML = array();
        $typeValueArray = typeCheck($instValue[1]);
        if($typeValueArray[0]!=="var") {
            exit(23);
        }
        $arraySecond = typeCheck($instValue[2]);
        if($arraySecond[0] == "label" || $arraySecond[0] =="type") {
            exit(23);
        }
        array_push($arrToXML, $instValue[0]);
        array_push($arrToXML, $typeValueArray);
        array_push($arrToXML, $arraySecond);
        addToXML($arrToXML);
        break;

    case "JUMPIFEQ":
    case "JUMPIFNEQ":

        $arrToXML = array();
        $typeValueArray = typeCheck($instValue[1]);
        if($typeValueArray[0]== "label" || $typeValueArray[0]== "type") {
            $typeValueArray[0] = "label";
        }
        else {
            exit(23);
        }
        $arraySecond = typeCheck($instValue[2]);
        $arrayThird = typeCheck($instValue[3]);

        if($arraySecond[0] == "label" || $arraySecond[0] =="type") {
            exit(23);
        }
        if($arrayThird[0] == "label" || $arrayThird[0] =="type") {
            exit(23);
        }
        array_push($arrToXML, $instValue[0]);
        array_push($arrToXML, $typeValueArray);
        array_push($arrToXML, $arraySecond);
        array_push($arrToXML, $arrayThird);
        addToXML($arrToXML);
        break;

    default:
        exit(22);
    }
}

// Function checks correctness of opcode and amount of arguments in instruction.
function countOfArguments($instructionWords): void
{

    $numberOfArguments = count($instructionWords)-1;

    if(!empty($instructionWords))
    {
        $opcode = strtoupper($instructionWords[0]);

        $zeroArgument = array("RETURN","PUSHFRAME","POPFRAME","CREATEFRAME","BREAK");
        $oneArgument = array("DEFVAR","CALL","LABEL","JUMP","EXIT","PUSHS","POPS","WRITE","DPRINT");
        $twoArguments = array("STRLEN","TYPE","MOVE","INT2CHAR","READ","NOT");
        $threeArguments = array("AND","OR","ADD","SUB","MUL","IDIV","LT","GT","EQ","CONCAT","STRI2INT",
           "GETCHAR","SETCHAR","JUMPIFEQ","JUMPIFNEQ");

        if(!empty($opcode))
        {    
            // Unknown opcode instructions
            if (!in_array($opcode, $zeroArgument) and !in_array($opcode, $oneArgument) and !in_array($opcode, $twoArguments) and !in_array($opcode, $threeArguments)) {
                exit(22);
            }
        };
    }
    // Check amount of the instruction argument
    switch ($numberOfArguments)
    {
        case 0:
            if (!in_array($opcode, $zeroArgument)) {
                exit(23);
            }
        break;
        
        case 1:
            if (!in_array($opcode, $oneArgument)) {
                exit(23);
            }
        break;
        
        case 2:
            if (!in_array($opcode, $twoArguments)) {
                exit(23);
            }
        break;
        
        case 3:
            if (!in_array($opcode, $threeArguments)) {
                exit(23);
            }
        break;
    // More than 3 argument
        default:
                exit(23);
    }
   
}

// The first function in the instruction processing.
// Function divides the input line into words, then calls another lexical and parsing function.
function instructionDivider($instructionLine): void
{
    $instructionParts = explode(' ', $instructionLine);
    countOfArguments($instructionParts);
    checkTypesOfArgument($instructionParts);
}

// Function add elements to the XML tree.
function addToXML($arr): void
{
    global $opcodeCount;
    global $program;
    $instruction[$opcodeCount] = $program->addChild('instruction');
    $instruction[$opcodeCount]->addAttribute('order', $opcodeCount);
    $instruction[$opcodeCount]->addAttribute('opcode', $arr[0]);
    if(count($arr)>1){
        for ($i = 1; $i<=count($arr)-1; $i++)
        {
            $argument[$i-2] = $instruction[$opcodeCount]->addChild("arg".($i), htmlspecialchars($arr[$i][1]));
            $type = $arr[$i][0];
            $argument[$i-2]->addAttribute('type', $type);
        }
    }
    $opcodeCount++;
}

// Starting point of the program. Then the program lines in IPPcode22 are loaded.
cmdArguments($argc, $argv);  

$stdin = fopen('php://stdin', 'r');
if(!$stdin) {
    exit(11);
}
while(($string = fgets(STDIN)) !== FALSE) {
    global $headerFound;
    $string = trim($string);
    // If the line contains only a comment
    if(empty($string) || $string[0] === "#")
        continue;
    $string = cutComments($string);
    $string = trim($string);
    if(!$headerFound) {
        if ($string !== ".IPPcode21") {
            exit(21);
        } else {
            $headerFound = true;
            continue;
        }
    }
    // We replace any white characters with space
    $string = preg_replace('/\s+/', ' ', $string);
    instructionDivider($string);
}

// Function processes arguments on the display line
function cmdArguments($argCounter, $arguments): void
{
    // check whether --help option is present, if so print help message and exit
    if ($argCounter == 2)
    {
      if ($arguments[1] === "--help")
      {
        printf("Usage:php parse.php [options] <inputFile> output.file\n\n");
        exit(0);
      }
      else
      {
        fwrite(STDERR,"Wrong option, try 'parser.php --help' for info, exiting\n");
        exit(10);
      }
    }
    // If in line more than 2 arguments
    else if ($argCounter > 2)
    {
      fwrite(STDERR,"Wrong option, try 'parser.php --help' for info, exiting\n");
      exit(10);
    }
}

// Function removes comment from the line
function cutComments(string $string): string
{
    if (strpos($string, '#') != NULL)
        return substr($string, 0, strpos($string, '#'));
    return $string;
}

// We store the resulting XML representation.
// Using this tutorial: https://stackoverflow.com/questions/8615422/php-xml-how-to-output-nice-format
$dom = new DOMDocument("1.0");
$dom->preserveWhiteSpace = false;
$dom->formatOutput = true;
$dom->loadXML($program->asXML());
fclose(STDIN); 
try{
    $dom->save("php://stdout");
}
catch (exception $e) {
    exit(12);
}
exit(0);
