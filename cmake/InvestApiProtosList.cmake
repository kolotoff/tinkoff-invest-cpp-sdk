set(InvestApiProtosDir "${CMAKE_SOURCE_DIR}/deps/investAPI/src/docs/contracts")
set(InvestApiProtosOutDir "${CMAKE_SOURCE_DIR}/src/protos")
file(GLOB InvestApiSrcProtos "${InvestApiProtosDir}/*.proto")

foreach (srcFilePath ${InvestApiSrcProtos})
    get_filename_component(filename ${srcFilePath} NAME)
    message("fix namespace in file ${filename}")    
    file(READ ${srcFilePath} infile)
    string(REPLACE "tinkoff.public.invest" "tinkoff.invest" fixed "${infile}")  
    file(WRITE "${InvestApiProtosOutDir}/${filename}" "${fixed}")
endforeach (srcFilePath ${APP_SOURCES})

file(GLOB InvestApiProtos "${InvestApiProtosOutDir}/*.proto")