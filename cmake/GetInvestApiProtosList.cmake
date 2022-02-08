set(InvestApiProtosDir "${CMAKE_CURRENT_LIST_DIR}/../deps/investAPI/src/docs/contracts")
set(InvestApiProtosOutDir "${CMAKE_CURRENT_LIST_DIR}/../src/protos")
file(GLOB InvestApiSrcProtos "${InvestApiProtosDir}/*.proto")

if(NOT InvestApiSrcProtos)
    message(FATAL_ERROR "Empty invest api src protos")
endif()

# Proto ����� �������� � ������ "package tinkoff.public.invest.api.*"
# ������� ������ �++ public, ������� �������� � namespace.
# �������� ������ �������� ������ p �� ��������� (public -> Public) ��� ���������� ����������.
# ��� �������������� ������������� � ��������(�������� ������ �� �������� �����) 
# ������������ RestoreProtosPackagePublicKeyword.cmake

foreach (srcFilePath ${InvestApiSrcProtos})
    get_filename_component(filename ${srcFilePath} NAME)
    message("fix namespace in file ${filename}")    
    file(READ ${srcFilePath} infile)
    string(REPLACE "tinkoff.public.invest" "tinkoff.Public.invest" fixed "${infile}")  
    file(WRITE "${InvestApiProtosOutDir}/${filename}" "${fixed}")
endforeach (srcFilePath ${InvestApiSrcProtos})

file(GLOB InvestApiProtos "${InvestApiProtosOutDir}/*.proto")

if(NOT InvestApiProtos)
    message(FATAL_ERROR "Empty invest api protos")
endif()