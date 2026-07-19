<?php
// +--------------------------------------------------------------------+ 
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2009 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+



class DiscompCommunicator {
	var $host;
	var $port;
	var $timeout;
	var $socket_handle;
	var $connected = false;
	var $authorized = false;
	var $errstr = "";
	var $errno  = "";
	var $protocol_msg_id = 1;
	var $debug = 0;
	var $login;
	var $password;

	function DiscompCommunicator ( $host = "127.0.0.1", $port = "5712", $timeout = 60 )
	{
		$this->host     = $host;
		$this->port     = $port;
		$this->timeout  = $timeout;
	}

	function setUser ( $login, $password_md5 ) {
		$this->login    = $login;
		$this->password = $password_md5;
//		if ( !$this->checkAuthorization ( ) ) {
//			$this->authorized = -1;
//		} else { 
//			$this->authorized = 1;
//		};
	}

	function connect ()
	{
		ob_start(NULL);
		$this->socket_handle = fsockopen($this->host, $this->port, $errno, $errstr, $this->timeout);
		$as = ob_get_contents();
		ob_end_clean();

		if(!$this->socket_handle) {
			$this->errstr = "Could not connect to $this->host:$this->port ($errstr)";
			$this->errno  = $errno;
			$this->connected = false;
		} else {
			$this->connected = true;
		};
	}

	/*
	function isAuthorized () 
	{
		if ( $this->authorized == 1 ) {
			return true;
		} else {
			return false;
		};
	}
	*/
	
	function request ( $code, $request_body="" )
	{
		$this->connect();

		if ( !$this->connected ) {
			if ( $this->errstr == "" ) {
				$this->errstr = "Not connected";
			};
			return -1;
		};

	//	if ( $this->authorized == -1 ) {
	//		$this->errstr = "User authorization failed";
	//		return -1;
	//	};


		$request_xml  = "<?xml version='1.0' encoding='UTF-8'?".">\n";
		$request_xml .= "<protocol msg_id='".$this->protocol_msg_id."'>\n";
		$request_xml .= "<request id='$code' login='".$this->login."' password='".$this->password."'>\n".$request_body."\n</request>\n";
		$request_xml .= "</protocol>\n";

		if ( $this->debug ) {
			echo "Request to server:\n". $request_xml;
		};

		$request_str = "S".$request_xml;
		$request_size = dechex(strlen($request_str));
		$request_size_str = str_repeat("0", 4-strlen($request_size)). $request_size;
		$request_data = $request_size_str. $request_str;

		fputs($this->socket_handle, $request_data );
		
		//TODO: not good
		$answer = "";
		while ( true ) {
			$block = fread($this->socket_handle, 1000);
			if ( strlen($block) <= 0 ) 
				break;
			$answer .= $block;
		}

		$answer = substr( $answer, 5 );
		if ( $this->debug ) {
			echo "\nServer answer:\n". $answer;
		};
		$dom = new DOMDocument();
		if ( !$dom->loadXML($answer) ) {
			echo "Received XML: <br>". preg_replace("/>/Umsi","&gt;", preg_replace("/</Umsi", "&lt;", $answer)); 
		};
		if ( $dom->getElementsByTagName("answer")->item(0)->getAttribute("auth") == -1 ) {
			$this->_errstr = "User authorization failed";
			return -1;
		};

		return $answer;
	}


	function checkAuthorization ( ) 
	{
		$xml = $this->request(1);
		if ( $xml == -1 ) { return false; };
		$dom = new DOMDocument();
		$dom->loadXML($xml);
		return $dom->getElementsByTagName("answer")->item(0)->getAttribute("user_type");
	}


	function getUsersList ( ) 
	{
		$xml = $this->request ( 4 );
		if ( $xml == -1 ) { return false; };
		$dom = new DOMDocument();
		$dom->loadXML($xml);
		$users_list = $dom->getElementsByTagName("user");
		for ( $i=0; $i<$users_list->length; $i++ ) {
			$item = $users_list->item($i);
			$user = array(
				"login"=>$item->getAttribute("login"),
				"type"=>$item->getAttribute("type")
			);
			$discomp_users_list_arr[] = $user;
		};
		return $discomp_users_list_arr;
	
	}



	function createNewUser ( $new_login, $new_password, $type ) 
	{
		$xml = $this->request ( 2, "<user new_login='".$new_login."' new_password='".md5($new_password)."' type='".$type."'/>");
		if ( $xml == -1 ) { return false; };
		$dom = new DOMDocument();
		$dom->loadXML($xml);
		return ($dom->getElementsByTagName("answer")->item(0)->getAttribute("status") == 1 ) ? true : false;
	}

	function changeUserPassword ( $new_password ) 
	{
		$xml = $this->request ( 3, "<user new_password='".md5($new_password)."'/>");
		if ( $xml == -1 ) { return false; };
		$dom = new DOMDocument();
		$dom->loadXML($xml);
		return ($dom->getElementsByTagName("answer")->item(0)->getAttribute("status") == 1 ) ? true : false;
	}


	function getNodesList ( )
	{
		$discomp_nodes_list_arr = array();

		$xml = $this->request (103);
		if ( $xml == -1 ) { return; };
		$dom = new DOMDocument();
		$dom->loadXML($xml);

		$nodes_list = $dom->getElementsByTagName("node");
		for ( $i=0; $i<$nodes_list->length; $i++ ) {
			$item = $nodes_list->item($i);
			$node = array(
				"id"=>$item->getAttribute("id"),
				"ip"=>$item->getAttribute("ip"),
				"name"=>$item->getAttribute("name"),
				"busy"=>$item->getAttribute("busy"),
				"process_name"=>$item->getAttribute("process_name"),
				"module_id"=>$item->getAttribute("module_id"),
				"module_list_num"=>$item->getAttribute("module_list_num"),
				"module_exec_time"=>$item->getAttribute("module_exec_time"),
				"module_name"=>$item->getAttribute("module_name"),
				"uptime"=>$item->getAttribute("uptime"),
				"ping"=>$item->getAttribute("ping")
			);
			$node['uptime'] = $this->calc_tl($node['uptime']);
			$discomp_nodes_list_arr[] = $node;
		};
		return $discomp_nodes_list_arr;
	}

	function getNodeInfo ( $id )
	{
		$discomp_nodes_list_arr = array();

		$xml = $this->request (102, "<node id='$id'/>");
		if ( $xml == -1 ) { return; };
		$dom = new DOMDocument();
		$dom->loadXML($xml);
		
		$nodes_list = $dom->getElementsByTagName("node");
		for ( $i=0; $i<$nodes_list->length; $i++ ) {
			$item = $nodes_list->item($i);
			$node = array(
				"id"=>$item->getAttribute("id"),
				"ip"=>$item->getAttribute("ip"),
				"name"=>$item->getAttribute("name"),
				"busy"=>$item->getAttribute("busy"),
				"process_id"=>$item->getAttribute("process_id"),
				"process_name"=>$item->getAttribute("process_name"),
				"module_name"=>$item->getAttribute("module_name"),
				"module_list_num"=>$item->getAttribute("module_list_num"),
				"module_exec_time"=>$item->getAttribute("module_exec_time"),
				"modules_list"=>split(",",$item->getAttribute("modules_list")),
				"failed_modules_list"=>split(",",$item->getAttribute("failed_modules_list")),
				"client_version"=>$item->getAttribute("client_version"),
				"sysinfo_xml"=>$item->nodeValue,
				"uptime"=>$item->getAttribute("uptime"),
				"ping"=>$item->getAttribute("ping")
			);
			$node['uptime'] = $this->calc_tl($node['uptime'], 1);

			foreach ( $node["modules_list"] as $key=>$val ) {
				if ( $val == "" ){
					unset($node["modules_list"][$key]);
				};
			};
			$node["modules_list_count"] = count($node["modules_list"]);
			foreach ( $node["failed_modules_list"] as $key=>$val ) {
				if ( $val == "" ){
					unset($node["failed_modules_list"][$key]);
				};
			};
			$node["failed_modules_list_count"] = count($node["failed_modules_list"]);

			$node['sysinfo'] = array();

			$node['sysinfo']['sys_name'] = $item->getElementsByTagName("sys_name")->item(0)->nodeValue;
			$node['sysinfo']['sys_version'] = $item->getElementsByTagName("sys_version")->item(0)->nodeValue;
			$node['sysinfo']['sys_machine'] = $item->getElementsByTagName("sys_machine")->item(0)->nodeValue;
			$node['sysinfo']['sys_description'] = $item->getElementsByTagName("sys_description")->item(0)->nodeValue;
			$node['sysinfo']['sys_vendor'] = $item->getElementsByTagName("sys_vendor")->item(0)->nodeValue;
			$node['sysinfo']['cpu_mhz'] = $item->getElementsByTagName("cpu_mhz")->item(0)->nodeValue;
			$node['sysinfo']['cpu_vendor'] = $item->getElementsByTagName("cpu_vendor")->item(0)->nodeValue;
			$node['sysinfo']['cpu_model'] = $item->getElementsByTagName("cpu_model")->item(0)->nodeValue;
			$node['sysinfo']['cpu_cores'] = $item->getElementsByTagName("cpu_cores")->item(0)->nodeValue;
			$node['sysinfo']['cpu_sockets'] = $item->getElementsByTagName("cpu_sockets")->item(0)->nodeValue;
			$node['sysinfo']['cpu_cache_size'] = $item->getElementsByTagName("cpu_cache_size")->item(0)->nodeValue;
			$node['sysinfo']['cpu_idle'] = $item->getElementsByTagName("cpu_idle")->item(0)->nodeValue;
			$node['sysinfo']['cpu_total'] = $item->getElementsByTagName("cpu_total")->item(0)->nodeValue;
			$node['sysinfo']['mem_total'] = $item->getElementsByTagName("mem_total")->item(0)->nodeValue;
			$node['sysinfo']['mem_free'] = $item->getElementsByTagName("mem_free")->item(0)->nodeValue;
			$node['sysinfo']['mem_used'] = $item->getElementsByTagName("mem_used")->item(0)->nodeValue;
			$node['sysinfo']['mem_free_pct'] = floor($item->getElementsByTagName("mem_free_pct")->item(0)->nodeValue);
			$node['sysinfo']['mem_used_pct'] = floor($item->getElementsByTagName("mem_used_pct")->item(0)->nodeValue);
			$node['sysinfo']['stat_uptime'] = $item->getElementsByTagName("stat_uptime")->item(0)->nodeValue;
			//$node['sysinfo']['stat_uptime'] = date("d H:i:s",$item->getElementsByTagName("stat_uptime")->item(0)->nodeValue);
			$node['sysinfo']['stat_loadavg1'] = $item->getElementsByTagName("stat_loadavg1")->item(0)->nodeValue;
			$node['sysinfo']['stat_loadavg2'] = $item->getElementsByTagName("stat_loadavg2")->item(0)->nodeValue;
			$node['sysinfo']['stat_loadavg3'] = $item->getElementsByTagName("stat_loadavg3")->item(0)->nodeValue;
			
			$node['sysinfo']['uptime'] = $this->calc_tl($node['sysinfo']['stat_uptime']);
			if ( $node['sysinfo']['cpu_total'] > 0 ) {
				$node['sysinfo']['cpu_idle_pct'] = floor ( $node['sysinfo']['cpu_idle']/ $node['sysinfo']['cpu_total'] * 100 );
			};
			$node['sysinfo']['mem_total_readable'] = $this->humanReadableSize($node['sysinfo']['mem_total']);

			return $node;	
			$discomp_nodes_list_arr[] = $node;
		};

		return $discomp_nodes_list_arr;
	}

	function updateNodeSysInfo ( $node_id ) 
	{
		$this->request (106, "<node id='$node_id'/>");
	}

	function deleteFailedModule ( $node_id, $module )
	{
		$this->request (107, "<node id='$node_id' module='$module'/>");
	}

	function installModule ( $node_id, $module ) 
	{
		$this->request (105, "<node id='$node_id'><module name='$module'/></node>");
	}

	function getProcessesList ( )
	{
		$discomp_processes_list_arr = array();

		$xml = $this->request (113);
		if ( $xml == -1 ) { return; };
		$dom = new DOMDocument();
		$dom->loadXML($xml);

		$nodes_list = $dom->getElementsByTagName("process");
		for ( $i=0; $i<$nodes_list->length; $i++ ) {
			$item = $nodes_list->item($i);
			$process = array(
				"id"=>$item->getAttribute("id"),
				"name"=>$item->getAttribute("name"),
				"user"=>$item->getAttribute("user"),
				"priority"=>$item->getAttribute("priority"),
				"completed"=>$item->getAttribute("completed"),
				"exec_time"=>$item->getAttribute("exec_time"),
				"current_stage"=>$item->getAttribute("current_stage")
			);
			$process['exec_time'] = $this->calc_tl($process['exec_time'], 1);
			$discomp_processes_list_arr[] = $process;
		};
		return $discomp_processes_list_arr;
	}


	function getProcessInfo ( $process_name )
	{
		$xml = $this->request (112,"<process name='$process_name'/>");
		if ( $xml == -1 ) { return; };
		$dom = new DOMDocument();
		$dom->loadXML($xml);
		return array(
			"id"=>$dom->getElementsByTagName("process")->item(0)->getAttribute("id"),
			"name"=>$dom->getElementsByTagName("process")->item(0)->getAttribute("name"),
			"completed"=>$dom->getElementsByTagName("process")->item(0)->getAttribute("completed"),
			"current_stage"=>$dom->getElementsByTagName("process")->item(0)->getAttribute("current_stage"),
		);
	}




	function startProcess ( $process_name )
	{
		$xml = $this->request (110, "<process name='$process_name'/>");
		if ( $xml == -1 ) { return; };
		$dom = new DOMDocument();
		$dom->loadXML($xml);
		return array(
			"status"=>$dom->getElementsByTagName("answer")->item(0)->getAttribute("status"),
			"msg"=>$dom->getElementsByTagName("answer")->item(0)->getAttribute("msg"),
		);
	}

	function stopProcess ( $process_name )
	{
		$xml = $this->request (111,"<process name='$process_name'/>");
		if ( $xml == -1 ) { return; };
		$dom = new DOMDocument();
		$dom->loadXML($xml);
		return array(
			"status"=>$dom->getElementsByTagName("answer")->item(0)->getAttribute("status"),
			"msg"=>$dom->getElementsByTagName("answer")->item(0)->getAttribute("msg"),
		);
	}



	function getMonitoringData ( $node_id, $interval )
	{
		$xml = $this->request (201,"<node id='$node_id' interval='$interval'/>");
		$xml = preg_replace("/<protocol[^>]*>/Umsi","",$xml);
		$xml = preg_replace("/<answer[^>]*>/Umsi","",$xml);
		$xml = preg_replace("/<\/answer>/Umsi","",$xml);
		$xml = preg_replace("/<\/protocol>/Umsi","",$xml);
		if ( $xml == -1 ) { return; };
		return $xml;
	}



	function humanReadableSize ( $file_size ) 
	{
		if ( $file_size < 1024 ) {
			return floor($file_size)." byte";
		} else if ( $file_size < 1024*1024 ) {
			return floor($file_size/1024)." Kb";
		} else  {
			return floor($file_size/(1024*1024))." Mb";
		};
	}




    function calc_tl($t, $show_seconds=false) {

        $sY = 31536000;
        $sW = 604800;
        $sD = 86400;
        $sH = 3600;
        $sM = 60;

        if($sT) {
            $t = ($sT - $t);
        }

        if($t <= 0) {
            $t = 0;
        }

        $bs[1] = ('1'^'9'); /* Backspace */
	$sel = 'y';
        switch(strtolower($sel)) {

            case 'y':
                $y = ((int)($t / $sY));
                $t = ($t - ($y * $sY));
		if ( $y != 0 ) {
                	$r['string'] .= "{$y} y ";
		};
                $r['years'] = $y;
            case 'w':
                $w = ((int)($t / $sW));
                $t = ($t - ($w * $sW));
		if ( $w != 0 ) {
               	 $r['string'] .= "{$w} weeks ";
		};
                $r['weeks'] = $w;
            case 'd':
                $d = ((int)($t / $sD));
                $t = ($t - ($d * $sD));
		if ( $d != 0 ) {
                	$r['string'] .= "{$d} d ";
		}
                $r['days'] = $d;
            case 'h':
                $h = ((int)($t / $sH));
                $t = ($t - ($h * $sH));
		if ( $h != 0 ) {
               	 $r['string'] .= "{$h} h. ";
		};
                $r['hours'] = $h;
            case 'm':
                $m = ((int)($t / $sM));
                $t = ($t - ($m * $sM));
		if ( $m != 0 ) {
                	$r['string'] .= "{$m} min. ";
		};
                $r['minutes'] = $m;
            case 's':
                $s = $t;
		if ( $show_seconds ) {
              		$r['string'] .= "{$s} sec.";
		};
                $r['seconds'] = $s;
            break;
            default:
                return $this->calc_tl($t);
            break;
        }

        return $r;
    }

};


