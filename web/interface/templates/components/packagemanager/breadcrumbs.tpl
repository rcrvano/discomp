<div style='padding:0px 0px 5px 10px'>
<a href='index.php?component=packagemanager'>Package manager</a> &raquo; 
{if $smarty.request.package != "" }
	Package: <a href='index.php?component=packagemanager&package={$smarty.request.package}'>{$smarty.request.package}</a> &raquo; 

	{if $smarty.request.module != "" }
		Module: <a href='index.php?component=packagemanager&module={$smarty.request.module}'>{$smarty.request.module}</a>  
	
	{/if}
	{if $smarty.request.parameter != "" }
		Parameter: <a href='index.php?component=packagemanager&parameter={$smarty.request.parameter}'>{$smarty.request.parameter}</a>  
	
	{/if}
	{if $smarty.request.scheme != "" }
		Scheme: <a href='index.php?component=packagemanager&scheme={$smarty.request.scheme}'>{$smarty.request.scheme}</a>  
	
	{/if}
{/if}
</div>
