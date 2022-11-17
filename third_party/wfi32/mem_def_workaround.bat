cp ./replace.vbs ../lwip/repo/lwip_test_2
cd ../lwip/repo/lwip_test_2/
for /r %%i in (*) do cscript replace.vbs %%i "mem_free(" "mem_free_l("
for /r %%i in (*) do cscript replace.vbs %%i "mem_free," "mem_free_l,"