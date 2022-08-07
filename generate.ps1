# $Env:HL_PERMIT_FAILED_UNROLL = "1"

function Generate-HalideFunction($OUTPUT_DIR, $ALGORITHM_NAME, $TARGET) {
    Write-Host "Adams2019"
    ./x64/Release/generator.exe -o $OUTPUT_DIR -g "${ALGORITHM_NAME}_gen" target="${TARGET}-no_runtime" -f "${ALGORITHM_NAME}_auto_scheduled_adams2019" -e static_library,h,schedule  auto_schedule=true -p $env:HALIDE_LIB_DIR/autoschedule_adams2019.dll -s Adams2019
    Write-Host "Li2018"
    ./x64/Release/generator.exe -o $OUTPUT_DIR -g "${ALGORITHM_NAME}_gen" target="${TARGET}-no_runtime" -f "${ALGORITHM_NAME}_auto_scheduled_li2018" -e static_library,h,schedule auto_schedule=true -p $env:HALIDE_LIB_DIR/autoschedule_li2018.dll -s Li2018
    Write-Host "Mullapudi2016"
    ./x64/Release/generator.exe -o $OUTPUT_DIR -g "${ALGORITHM_NAME}_gen" target="${TARGET}-no_runtime" -f "${ALGORITHM_NAME}_auto_scheduled_mullapudi2016" -e static_library,h,schedule auto_schedule=true -p $env:HALIDE_LIB_DIR/autoschedule_mullapudi2016.dll -s Mullapudi2016 
}

$OUTPUT_DIR="./generator/dest/host"
$TARGET="host"

if (-not (Test-Path $OUTPUT_DIR)) {
    mkdir -p $OUTPUT_DIR
}

$ALGORITHM_NAME="halide_runtime"
Write-Host $ALGORITHM_NAME
./x64/Release/generator.exe -o $OUTPUT_DIR -g "${ALGORITHM_NAME}_gen" -f ${ALGORITHM_NAME} -e static_library,h target="${TARGET}"

$ALGORITHM_NAME_LIST = @(
    "halide_dcci_search"
    "halide_dcci"
    # "box_filter"
)

foreach ( $ALGORITHM_NAME in $ALGORITHM_NAME_LIST)
{
    Write-Host $ALGORITHM_NAME
    Generate-HalideFunction $OUTPUT_DIR $ALGORITHM_NAME $TARGET
}
