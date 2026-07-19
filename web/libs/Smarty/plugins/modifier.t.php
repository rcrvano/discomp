<?php 
// +--------------------------------------------------------------------+
// | TelinWeb Desktop                                                   |
// +--------------------------------------------------------------------+
// | Copyright (C) 2005-2008 TelinWeb Inc.                              |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author : Anton Perkov <anton@artwhere.be> <unquot@yandex.ru>       |
// +--------------------------------------------------------------------+

function smarty_modifier_t($q)
 {
	return htmlspecialchars($q, ENT_QUOTES, 'UTF-8');
 }
