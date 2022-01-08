file(GLOB fileList "${GENERATED_PROTOS_OUT_DIR}/*.cc" "${GENERATED_PROTOS_OUT_DIR}/*.h")

message("GENERATED_PROTOS_OUT_DIR ${GENERATED_PROTOS_OUT_DIR}")    

foreach (srcFilePath ${fileList})
    get_filename_component(filename ${srcFilePath} NAME)
    message("fix package name in file ${filename}")    
    file(READ ${srcFilePath} infile)
    string(REPLACE "tinkoff.Public.invest" "tinkoff.public.invest" fixed1 "${infile}")  
    string(REPLACE "tinkoff.P" "tinkoff.p" fixed2 "${fixed1}")  
    string(REPLACE "Public.invest" "public.invest" fixed "${fixed2}")
    file(WRITE "${srcFilePath}" "${fixed}")
endforeach (srcFilePath ${fileList})