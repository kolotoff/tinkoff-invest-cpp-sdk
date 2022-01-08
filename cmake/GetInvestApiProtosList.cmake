set(InvestApiProtosDir "${CMAKE_SOURCE_DIR}/deps/investAPI/src/docs/contracts")
set(InvestApiProtosOutDir "${CMAKE_SOURCE_DIR}/src/protos")
file(GLOB InvestApiSrcProtos "${InvestApiProtosDir}/*.proto")

# Proto файлы содержат в строке "package tinkoff.public.invest.api.*"
# ключево словое С++ public, которое попадает в namespace.
# Заменяем первую строчную буквку p на заглавную (public -> Public) для нормальной компиляции.
# Для восстановления совместимости с сервером(обратной замены на строчную букву) 
# используется RestoreProtosPackagePublicKeyword.cmake

foreach (srcFilePath ${InvestApiSrcProtos})
    get_filename_component(filename ${srcFilePath} NAME)
    message("fix namespace in file ${filename}")    
    file(READ ${srcFilePath} infile)
    string(REPLACE "tinkoff.public.invest" "tinkoff.Public.invest" fixed "${infile}")  
    file(WRITE "${InvestApiProtosOutDir}/${filename}" "${fixed}")
endforeach (srcFilePath ${InvestApiSrcProtos})

file(GLOB InvestApiProtos "${InvestApiProtosOutDir}/*.proto")