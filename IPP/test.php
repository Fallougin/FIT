<?php

try {
    $dom = new DOMDocument('1.0');
} catch (DOMException) {
    exit(99);
}

$testCounter = 0;
$testsPassed = 0;
$testsFailed = 0;

$css_text = '';
$css_text .= 'body{ width=100%; margin-top:50px; justify-content: center;} ';
$css_text .= '.mainTable{ margin-left:20px; margin-bottom:20px; width=90%;} ';
$css_text .= '.mainTable th{text-decoration:underline; width=200px;} ';
$css_text .= '.mainTable td{width=200px;} ';
$css_text .= '.mainTable td:first-child{text-align:left;color:black;} ';
$css_text .= '.pass{ text-align:left; color:green; font-weight:bold;} ';
$css_text .= '.fail{text-align:left;color:red;widh=200px; font-weight:bold;} ';
$css_text .= '.total{margin-left:100px;text-align:left;color:black;width=200px; font-weight:bold;};';


$style = $dom->createElement('style', $css_text);

$domAttribute = $dom->createAttribute('type');
$domAttribute->value = 'text/css';
$style->appendChild($domAttribute);
$dom->appendChild($style);

$table = $dom->createElement('table');
$domAttribute = $dom->createAttribute('class');
$domAttribute->value = 'mainTable';

$options = getopt("", [
    "help::",
    "directory::",
    "recursive::",
    "parse-script::",
    "int-script::",
    "parse-only::",
    "int-only::",
    "jexampath::",
    "noclean::",
]);

$path = shell_exec('pwd');
$path = rtrim($path, "\n");


$recursive = array_key_exists('recursive', $options);
$directory = array_key_exists('directory', $options) ? $options['directory'] : $path;
$parseScript = array_key_exists('parse-script', $options) ? $options['parse-script'] : './parse.php';
$intScript = array_key_exists('int-script', $options) ? $options['int-script'] : './interpret.py';
$parseOnly = array_key_exists('parse-only', $options);
$intOnly = array_key_exists('int-only', $options);
$jExamPath = array_key_exists('jexampath', $options) ? $options['jexampath'] : '/pub/courses/ipp/jexamxml/jexamxml.jar';
$noClean = array_key_exists('noclean', $options);
$help = array_key_exists('help', $options);


/** Check params*/

if ($jExamPath && $intOnly) {
    exit(10);
}

if ($help) {
    echo ('Test frame for parse.php and interpret.py');
    echo ('Usage:  (all parameters are optional)');
    echo ('--directory=<path to test folder>');
    echo ('--parse-script=<path to parse.php>');
    echo ('--int-script=<path to interpret.py>');
    echo ('--jexampath=<path to jexamxml.jar>');
    echo ('--parse-only');
    echo ('--int-only');
    echo ('--recursive');
    echo ('--noclean - will not delete intermediate filesjexampath');
}

if (!file_exists('./tmpTest')) {
    mkdir('./tmpTest', 0777, true);
}

/** Main test function. */

mainTest();

function mainTest(): void
{
    global $directory;
    global $recursive;
    global $noClean;

    global $testCounter;
    global $testsPassed;
    global $testFailed;

    global $intScript;
    global $parseScript;
    global $parseOnly;
    global $intOnly;

    $folders = getFolders($directory, $recursive);
    foreach ($folders as $folder) {
        $testCounter++;
        $fileName = pathinfo($folder, PATHINFO_FILENAME);
        $fileDir = pathinfo($folder, PATHINFO_DIRNAME) . '/';
        $tmpTestFile = './tmpTest/' . (string) $fileName;

        $srcFileName = $fileName . '.src';
        $rcFileName = $fileName . '.rc';
        $inFileName = $fileName . '.in';
        $outFileName = $fileName . '.out';

        if (!file_exists($fileDir.$rcFileName)) {
            $rcFile = fopen($fileDir.$inFileName, "w");
            if (!$rcFile) {
                exit(12);
            }
            file_put_contents($fileDir.$rcFileName, 0);
        }

        if (!file_exists($fileDir.$inFileName)) {
            $inFileName = fopen($fileDir.$inFileName, "w");
            if (!$inFileName) {
                exit(12);
            }
        }

        if (!file_exists($fileDir.$outFileName)) {
            $outFileName = fopen($fileDir.$outFileName, "w");
            if (!$outFileName) {
                exit(12);
            }
        }

        $outTest = $tmpTestFile . '.testOut';
        if (!$parseOnly) {
            $intString = 'python3.8 ' . $intScript .' --source='.$fileDir.$srcFileName.' >'.$outTest .' --input='. $fileDir.$inFileName;
            $testRcCode = (int) shell_exec($intString .'; echo $?');
        }
        if (!$intOnly) {
            $parseString = 'php8.1 ' . $parseScript . ' < ' . $fileDir.$srcFileName . ' 2>&1 > ' . $outTest;
            $testRcCode = (int) shell_exec($parseString . '; echo $?');
        }
        if (!$parseOnly && !$intOnly) {
            $parseString = 'php8.1 ' . $parseScript . ' < ' . $fileDir.$srcFileName . ' 2>&1 > ' . $outTest;
            $intOutput = $tmpTestFile . '.intOut';
            if ($outTest !== false){
                $intString = 'python3.8 ' . $intScript .' --source '.$outTest.' >'.$intOutput .' --input '. $fileDir.$inFileName;
                $testRcCode = (int) shell_exec($intString .'; echo $?');
                $rcFile = fopen($fileDir.$rcFileName, "r+");
                $referenceRcCode = (int) fgets($rcFile);
                compareOutputs($testRcCode, $referenceRcCode, $intOutput, $fileDir, $fileName);
                continue;
            }
        }
        
        $rcFile = fopen($fileDir.$rcFileName, "r+");
        $referenceRcCode = fgets($rcFile);
        compareOutputs($testRcCode, $referenceRcCode, $outTest, $fileDir, $fileName);
    }

    if ($noClean === false) {
        exec('rm -rf ./tmpTest', $output, $code);
    }
}

function compareOutputs(int $testRcCode, int $referenceRcCode, $outTest , $fileDir, $fileName): void
{
    global $parseOnly;
    global $jExamPath;

    global $testsPassed;
    global $testsFailed;

    if ($testRcCode === (int) $referenceRcCode && $testRcCode !== 0) {
        $testsPassed++;
        addTableRow($fileDir.$fileName, true);
    }
    elseif ($testRcCode === (int) $referenceRcCode && $testRcCode === 0) {
        $diff = "";
        if ($parseOnly) {
            try {
                $code = exec("java -jar ".$jExamPath." ". $outTest . " " .$fileDir.$fileName.".out ", $jExamOut, $jExamStatus);
                
            } catch (Exception $e) {
                exit(11);
            }
            
            $diff = $jExamStatus !== 0 ? $jExamStatus : "";
        } else {
            try{
                $diff = shell_exec("diff ". $outTest . " " .$fileDir.$outFileName);
                }
                catch (Exception $e) {
                    exit(11);
                }  
        }

        if ($diff === "") {
            $testsPassed++;
            addTableRow($fileDir.$fileName, true);
        }
        else {
            $testsFailed++;
            addTableRow($fileDir.$fileName, false, null, null, $diff);
        }
    }
    else {
        $testsFailed++;
        addTableRow($fileDir.$fileName, false, $testRcCode, $referenceRcCode);
    }
}

function getFolders(string $directory, bool $recursive): array
{

    if ($recursive){
        try{
            exec("find " . $directory.'/' . " -regex '.*\.src$'", $folders);
        }
        catch (Exception $e) {
            exit(11);
        }
    }
    else{
        try{
            exec("find " . $directory.'/' . " -maxdepth 1 -regex '.*\.src$'", $folders);
        }
        catch (Exception $e) {
            exit(11);
        }
    }
    return $folders;
}

function  addTableRow(string $testName, bool $passed, ?string $testOutput = null, ?string $referenceOutput = null, ?string $diff = null) {
    global $dom;
    global $table;
    $tr = $dom->createElement('tr');
    $table->appendChild($tr);

    if ($passed) {
        $td = $dom->createElement('td', "PASS");
        $td->setAttribute("class", "pass");
        $tr->appendChild($td);
        $td = $dom->createElement('td', $testName);
        $td->setAttribute("class", "pass");
        $tr->appendChild($td);
    } else {
        $td = $dom->createElement('td', "FAIL");
        $tr->appendChild($td);
        $td->setAttribute("class", "fail");
        $td = $dom->createElement('td', $testName);
        $td->setAttribute("class", "fail");
        $tr->appendChild($td);
        if ($diff === null) {
            $td = $dom->createElement('td', 'Your RC code is '. $testOutput . ' (Reference rc code is ' . $referenceOutput.')');
            $td->setAttribute("class", "fail");
            $td->setAttribute("style", "font-family: 'Courier New', monospace; font-weight:1; color:crimson;");
            $tr->appendChild($td);
        } else {
            $td = $dom->createElement('td', 'diffs are different '. $diff);
            $td->setAttribute("class", "fail");
            $td->setAttribute("style", "font-family: 'Courier New', monospace; font-weight:1; color:crimson;");
            $tr->appendChild($td);
        }
    }
}

$tr = $dom->createElement('tr');
$table->appendChild($tr);
$td = $dom->createElement('td', "TOTAL: ".$testCounter);
$tr->appendChild($td);
$td = $dom->createElement('td', "Failed:  ". (string) $testsFailed);

if($testsFailed !== 0){
    $td->setAttribute("class", "fail");
}
$tr->appendChild($td);
$td = $dom->createElement('td', "   Passed: ".$testsPassed);
$td->setAttribute("class", "pass");
$tr->appendChild($td);
$tr->setAttribute("class", "total");

$dom->appendChild($table);
echo $dom->saveHTML();