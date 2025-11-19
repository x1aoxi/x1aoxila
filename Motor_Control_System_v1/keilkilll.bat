del *.bak /s
del *.ddk /s
del *.edk /s
del *.lst /s
del *.lnp /s
del *.mpf /s
del *.mpj /s
del *.obj /s
del *.omf /s
::del *.opt /s  ::不允许删除JLINK的设置
del *.plg /s
del *.rpt /s
del *.tmp /s
del *.__i /s
del *.crf /s
del *.o /s
del *.d /s
del *.axf /s
del *.tra /s
del *.dep /s           
del JLinkLog.txt /s
del *.iex /s
del *.htm /s
del *.sct /s
del *.map /s
del *.orig /s

::新增的清理项
del *.hex /s
del *.bin /s
del *.uvguix.* /s
::del *.uvoptx /s
del *.scvd /s
::del *.dbgconf /s   ::保留调试配置
del EventRecorderStub.* /s
del *.i /s
del *.build_log.htm /s

::清理MDK生成的文件夹
rd /s /q Objects 2>nul
rd /s /q Listings 2>nul
::rd /s /q DebugConfig 2>nul  ::保留调试配置
::rd /s /q RTE 2>nul          ::保留运行时环境

echo 清理完成!
pause
