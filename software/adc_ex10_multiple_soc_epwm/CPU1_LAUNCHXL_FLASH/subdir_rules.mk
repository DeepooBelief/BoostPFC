################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
adc_ex10_multiple_soc_epwm.obj: ../adc_ex10_multiple_soc_epwm.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs2010/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --idiv_support=idiv0 --tmu_support=tmu0 -O2 --fp_mode=relaxed --include_path="C:/Users/Will/Downloads/PWM_WORK/PWM_WORK/adc_ex10_multiple_soc_epwm" --include_path="C:/ti/C2000Ware_5_04_00_00" --include_path="C:/Users/Will/Downloads/PWM_WORK/PWM_WORK/adc_ex10_multiple_soc_epwm/device" --include_path="C:/ti/C2000Ware_5_04_00_00/driverlib/f28002x/driverlib/" --include_path="C:/ti/ccs2010/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --define=DEBUG --define=_FLASH --define=_LAUNCHXL_F280025C --printf_support=full --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" --include_path="C:/Users/Will/Downloads/PWM_WORK/PWM_WORK/adc_ex10_multiple_soc_epwm/CPU1_LAUNCHXL_FLASH/syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1279252639: ../adc_ex10_multiple_soc_epwm.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs2010/ccs/utils/sysconfig_1.23.0/sysconfig_cli.bat" --script "C:/Users/Will/Downloads/PWM_WORK/PWM_WORK/adc_ex10_multiple_soc_epwm/adc_ex10_multiple_soc_epwm.syscfg" -o "syscfg" -s "C:/ti/C2000Ware_5_04_00_00/.metadata/sdk.json" -d "F28002x" -p "80QFP" -r "F28002x_80QFP" --compiler ccs
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/board.c: build-1279252639 ../adc_ex10_multiple_soc_epwm.syscfg
syscfg/board.h: build-1279252639
syscfg/board.cmd.genlibs: build-1279252639
syscfg/board.opt: build-1279252639
syscfg/board.json: build-1279252639
syscfg/pinmux.csv: build-1279252639
syscfg/adc.dot: build-1279252639
syscfg/c2000ware_libraries.cmd.genlibs: build-1279252639
syscfg/c2000ware_libraries.opt: build-1279252639
syscfg/c2000ware_libraries.c: build-1279252639
syscfg/c2000ware_libraries.h: build-1279252639
syscfg/clocktree.h: build-1279252639
syscfg: build-1279252639

syscfg/%.obj: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs2010/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --idiv_support=idiv0 --tmu_support=tmu0 -O2 --fp_mode=relaxed --include_path="C:/Users/Will/Downloads/PWM_WORK/PWM_WORK/adc_ex10_multiple_soc_epwm" --include_path="C:/ti/C2000Ware_5_04_00_00" --include_path="C:/Users/Will/Downloads/PWM_WORK/PWM_WORK/adc_ex10_multiple_soc_epwm/device" --include_path="C:/ti/C2000Ware_5_04_00_00/driverlib/f28002x/driverlib/" --include_path="C:/ti/ccs2010/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --define=DEBUG --define=_FLASH --define=_LAUNCHXL_F280025C --printf_support=full --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="syscfg/$(basename $(<F)).d_raw" --include_path="C:/Users/Will/Downloads/PWM_WORK/PWM_WORK/adc_ex10_multiple_soc_epwm/CPU1_LAUNCHXL_FLASH/syscfg" --obj_directory="syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


