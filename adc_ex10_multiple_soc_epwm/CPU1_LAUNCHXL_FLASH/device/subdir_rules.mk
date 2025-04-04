################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
device/%.obj: ../device/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs2010/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --idiv_support=idiv0 --tmu_support=tmu0 -O2 --fp_mode=relaxed --include_path="C:/Users/Will/Downloads/PWM_WORK/PWM_WORK/adc_ex10_multiple_soc_epwm" --include_path="C:/ti/C2000Ware_5_04_00_00" --include_path="C:/Users/Will/Downloads/PWM_WORK/PWM_WORK/adc_ex10_multiple_soc_epwm/device" --include_path="C:/ti/C2000Ware_5_04_00_00/driverlib/f28002x/driverlib/" --include_path="C:/ti/ccs2010/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --define=DEBUG --define=_FLASH --define=_LAUNCHXL_F280025C --printf_support=full --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="device/$(basename $(<F)).d_raw" --include_path="C:/Users/Will/Downloads/PWM_WORK/PWM_WORK/adc_ex10_multiple_soc_epwm/CPU1_LAUNCHXL_FLASH/syscfg" --obj_directory="device" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

device/%.obj: ../device/%.asm $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs2010/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --idiv_support=idiv0 --tmu_support=tmu0 -O2 --fp_mode=relaxed --include_path="C:/Users/Will/Downloads/PWM_WORK/PWM_WORK/adc_ex10_multiple_soc_epwm" --include_path="C:/ti/C2000Ware_5_04_00_00" --include_path="C:/Users/Will/Downloads/PWM_WORK/PWM_WORK/adc_ex10_multiple_soc_epwm/device" --include_path="C:/ti/C2000Ware_5_04_00_00/driverlib/f28002x/driverlib/" --include_path="C:/ti/ccs2010/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --define=DEBUG --define=_FLASH --define=_LAUNCHXL_F280025C --printf_support=full --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="device/$(basename $(<F)).d_raw" --include_path="C:/Users/Will/Downloads/PWM_WORK/PWM_WORK/adc_ex10_multiple_soc_epwm/CPU1_LAUNCHXL_FLASH/syscfg" --obj_directory="device" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


