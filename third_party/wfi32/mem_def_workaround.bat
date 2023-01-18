xcopy replace.vbs ..\lwip\repo\lwip
cd ../lwip/repo/lwip/
for /r %%i in (*) do cscript replace.vbs %%i "mem_free(" "mem_free_l("
for /r %%i in (*) do cscript replace.vbs %%i "mem_free," "mem_free_l,"