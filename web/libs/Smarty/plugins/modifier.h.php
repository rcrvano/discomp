<?
// +--------------------------------------------------------------------+
// | TelinWeb Desktop                                                   |
// +--------------------------------------------------------------------+
// | Copyright (C) 2005-2008 TelinWeb Inc.                              |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author : Anton Perkov <anton@artwhere.be> <unquot@yandex.ru>       |
// +--------------------------------------------------------------------+

require_once('classes/filters.php');

function smarty_modifier_h($q)
 {
	return Filters::pure_html($q);
 }
