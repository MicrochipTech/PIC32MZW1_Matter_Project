#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-pic32mz_w1_curiosity.mk)" "nbproject/Makefile-local-pic32mz_w1_curiosity.mk"
include nbproject/Makefile-local-pic32mz_w1_curiosity.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=pic32mz_w1_curiosity
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/pic32mz_w1_curiosity.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/pic32mz_w1_curiosity.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../src/config/pic32mz_w1_curiosity/bootloader/bootloader_uart.c ../src/config/pic32mz_w1_curiosity/bootloader/bootloader_common.c ../src/config/pic32mz_w1_curiosity/peripheral/clk/plib_clk.c ../src/config/pic32mz_w1_curiosity/peripheral/coretimer/plib_coretimer.c ../src/config/pic32mz_w1_curiosity/peripheral/evic/plib_evic.c ../src/config/pic32mz_w1_curiosity/peripheral/gpio/plib_gpio.c ../src/config/pic32mz_w1_curiosity/peripheral/nvm/plib_nvm.c ../src/config/pic32mz_w1_curiosity/peripheral/spi/spi_master/plib_spi1_master.c ../src/config/pic32mz_w1_curiosity/peripheral/uart/plib_uart1.c ../src/config/pic32mz_w1_curiosity/sst26/app_sst26vf.c ../src/config/pic32mz_w1_curiosity/sst26/ext_flash.c ../src/config/pic32mz_w1_curiosity/initialization.c ../src/config/pic32mz_w1_curiosity/pmu_init.c ../src/config/pic32mz_w1_curiosity/interrupts.c ../src/config/pic32mz_w1_curiosity/exceptions.c ../src/main.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1611361252/bootloader_uart.o ${OBJECTDIR}/_ext/1611361252/bootloader_common.o ${OBJECTDIR}/_ext/1481979610/plib_clk.o ${OBJECTDIR}/_ext/1145473014/plib_coretimer.o ${OBJECTDIR}/_ext/1303341575/plib_evic.o ${OBJECTDIR}/_ext/1303395403/plib_gpio.o ${OBJECTDIR}/_ext/1481968727/plib_nvm.o ${OBJECTDIR}/_ext/521489940/plib_spi1_master.o ${OBJECTDIR}/_ext/1303798346/plib_uart1.o ${OBJECTDIR}/_ext/1903958431/app_sst26vf.o ${OBJECTDIR}/_ext/1903958431/ext_flash.o ${OBJECTDIR}/_ext/1737632808/initialization.o ${OBJECTDIR}/_ext/1737632808/pmu_init.o ${OBJECTDIR}/_ext/1737632808/interrupts.o ${OBJECTDIR}/_ext/1737632808/exceptions.o ${OBJECTDIR}/_ext/1360937237/main.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1611361252/bootloader_uart.o.d ${OBJECTDIR}/_ext/1611361252/bootloader_common.o.d ${OBJECTDIR}/_ext/1481979610/plib_clk.o.d ${OBJECTDIR}/_ext/1145473014/plib_coretimer.o.d ${OBJECTDIR}/_ext/1303341575/plib_evic.o.d ${OBJECTDIR}/_ext/1303395403/plib_gpio.o.d ${OBJECTDIR}/_ext/1481968727/plib_nvm.o.d ${OBJECTDIR}/_ext/521489940/plib_spi1_master.o.d ${OBJECTDIR}/_ext/1303798346/plib_uart1.o.d ${OBJECTDIR}/_ext/1903958431/app_sst26vf.o.d ${OBJECTDIR}/_ext/1903958431/ext_flash.o.d ${OBJECTDIR}/_ext/1737632808/initialization.o.d ${OBJECTDIR}/_ext/1737632808/pmu_init.o.d ${OBJECTDIR}/_ext/1737632808/interrupts.o.d ${OBJECTDIR}/_ext/1737632808/exceptions.o.d ${OBJECTDIR}/_ext/1360937237/main.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1611361252/bootloader_uart.o ${OBJECTDIR}/_ext/1611361252/bootloader_common.o ${OBJECTDIR}/_ext/1481979610/plib_clk.o ${OBJECTDIR}/_ext/1145473014/plib_coretimer.o ${OBJECTDIR}/_ext/1303341575/plib_evic.o ${OBJECTDIR}/_ext/1303395403/plib_gpio.o ${OBJECTDIR}/_ext/1481968727/plib_nvm.o ${OBJECTDIR}/_ext/521489940/plib_spi1_master.o ${OBJECTDIR}/_ext/1303798346/plib_uart1.o ${OBJECTDIR}/_ext/1903958431/app_sst26vf.o ${OBJECTDIR}/_ext/1903958431/ext_flash.o ${OBJECTDIR}/_ext/1737632808/initialization.o ${OBJECTDIR}/_ext/1737632808/pmu_init.o ${OBJECTDIR}/_ext/1737632808/interrupts.o ${OBJECTDIR}/_ext/1737632808/exceptions.o ${OBJECTDIR}/_ext/1360937237/main.o

# Source Files
SOURCEFILES=../src/config/pic32mz_w1_curiosity/bootloader/bootloader_uart.c ../src/config/pic32mz_w1_curiosity/bootloader/bootloader_common.c ../src/config/pic32mz_w1_curiosity/peripheral/clk/plib_clk.c ../src/config/pic32mz_w1_curiosity/peripheral/coretimer/plib_coretimer.c ../src/config/pic32mz_w1_curiosity/peripheral/evic/plib_evic.c ../src/config/pic32mz_w1_curiosity/peripheral/gpio/plib_gpio.c ../src/config/pic32mz_w1_curiosity/peripheral/nvm/plib_nvm.c ../src/config/pic32mz_w1_curiosity/peripheral/spi/spi_master/plib_spi1_master.c ../src/config/pic32mz_w1_curiosity/peripheral/uart/plib_uart1.c ../src/config/pic32mz_w1_curiosity/sst26/app_sst26vf.c ../src/config/pic32mz_w1_curiosity/sst26/ext_flash.c ../src/config/pic32mz_w1_curiosity/initialization.c ../src/config/pic32mz_w1_curiosity/pmu_init.c ../src/config/pic32mz_w1_curiosity/interrupts.c ../src/config/pic32mz_w1_curiosity/exceptions.c ../src/main.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-pic32mz_w1_curiosity.mk ${DISTDIR}/pic32mz_w1_curiosity.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MZ1025W104132
MP_LINKER_FILE_OPTION=,--script="..\src\config\pic32mz_w1_curiosity\btl.ld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1611361252/bootloader_uart.o: ../src/config/pic32mz_w1_curiosity/bootloader/bootloader_uart.c  .generated_files/flags/pic32mz_w1_curiosity/bd13b6a2da0a008e787cac8064d9101445ab89a7 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1611361252" 
	@${RM} ${OBJECTDIR}/_ext/1611361252/bootloader_uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/1611361252/bootloader_uart.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1611361252/bootloader_uart.o.d" -o ${OBJECTDIR}/_ext/1611361252/bootloader_uart.o ../src/config/pic32mz_w1_curiosity/bootloader/bootloader_uart.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1611361252/bootloader_common.o: ../src/config/pic32mz_w1_curiosity/bootloader/bootloader_common.c  .generated_files/flags/pic32mz_w1_curiosity/7c0cb1d43ae88f16daae62c0ccc37fd8aa9f7a89 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1611361252" 
	@${RM} ${OBJECTDIR}/_ext/1611361252/bootloader_common.o.d 
	@${RM} ${OBJECTDIR}/_ext/1611361252/bootloader_common.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1611361252/bootloader_common.o.d" -o ${OBJECTDIR}/_ext/1611361252/bootloader_common.o ../src/config/pic32mz_w1_curiosity/bootloader/bootloader_common.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1481979610/plib_clk.o: ../src/config/pic32mz_w1_curiosity/peripheral/clk/plib_clk.c  .generated_files/flags/pic32mz_w1_curiosity/50552b4151b60d70463327186a21e32c80e08d7a .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1481979610" 
	@${RM} ${OBJECTDIR}/_ext/1481979610/plib_clk.o.d 
	@${RM} ${OBJECTDIR}/_ext/1481979610/plib_clk.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1481979610/plib_clk.o.d" -o ${OBJECTDIR}/_ext/1481979610/plib_clk.o ../src/config/pic32mz_w1_curiosity/peripheral/clk/plib_clk.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1145473014/plib_coretimer.o: ../src/config/pic32mz_w1_curiosity/peripheral/coretimer/plib_coretimer.c  .generated_files/flags/pic32mz_w1_curiosity/ee396fb8ef0b2a6e85b4a16fd32d48c3197bcf02 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1145473014" 
	@${RM} ${OBJECTDIR}/_ext/1145473014/plib_coretimer.o.d 
	@${RM} ${OBJECTDIR}/_ext/1145473014/plib_coretimer.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1145473014/plib_coretimer.o.d" -o ${OBJECTDIR}/_ext/1145473014/plib_coretimer.o ../src/config/pic32mz_w1_curiosity/peripheral/coretimer/plib_coretimer.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1303341575/plib_evic.o: ../src/config/pic32mz_w1_curiosity/peripheral/evic/plib_evic.c  .generated_files/flags/pic32mz_w1_curiosity/c280318c3a133ed61e3f1d3eacdf9ba6396a7142 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1303341575" 
	@${RM} ${OBJECTDIR}/_ext/1303341575/plib_evic.o.d 
	@${RM} ${OBJECTDIR}/_ext/1303341575/plib_evic.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1303341575/plib_evic.o.d" -o ${OBJECTDIR}/_ext/1303341575/plib_evic.o ../src/config/pic32mz_w1_curiosity/peripheral/evic/plib_evic.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1303395403/plib_gpio.o: ../src/config/pic32mz_w1_curiosity/peripheral/gpio/plib_gpio.c  .generated_files/flags/pic32mz_w1_curiosity/a821cac48cdbc958b2632136a39bd882318280a0 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1303395403" 
	@${RM} ${OBJECTDIR}/_ext/1303395403/plib_gpio.o.d 
	@${RM} ${OBJECTDIR}/_ext/1303395403/plib_gpio.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1303395403/plib_gpio.o.d" -o ${OBJECTDIR}/_ext/1303395403/plib_gpio.o ../src/config/pic32mz_w1_curiosity/peripheral/gpio/plib_gpio.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1481968727/plib_nvm.o: ../src/config/pic32mz_w1_curiosity/peripheral/nvm/plib_nvm.c  .generated_files/flags/pic32mz_w1_curiosity/21ffcb9077f5e0cfe489d6363ded41c629ccdd72 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1481968727" 
	@${RM} ${OBJECTDIR}/_ext/1481968727/plib_nvm.o.d 
	@${RM} ${OBJECTDIR}/_ext/1481968727/plib_nvm.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1481968727/plib_nvm.o.d" -o ${OBJECTDIR}/_ext/1481968727/plib_nvm.o ../src/config/pic32mz_w1_curiosity/peripheral/nvm/plib_nvm.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/521489940/plib_spi1_master.o: ../src/config/pic32mz_w1_curiosity/peripheral/spi/spi_master/plib_spi1_master.c  .generated_files/flags/pic32mz_w1_curiosity/964828869c05c73a9a53c72724b212631bfdaef5 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/521489940" 
	@${RM} ${OBJECTDIR}/_ext/521489940/plib_spi1_master.o.d 
	@${RM} ${OBJECTDIR}/_ext/521489940/plib_spi1_master.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/521489940/plib_spi1_master.o.d" -o ${OBJECTDIR}/_ext/521489940/plib_spi1_master.o ../src/config/pic32mz_w1_curiosity/peripheral/spi/spi_master/plib_spi1_master.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1303798346/plib_uart1.o: ../src/config/pic32mz_w1_curiosity/peripheral/uart/plib_uart1.c  .generated_files/flags/pic32mz_w1_curiosity/87ab04340cd3a1fec502a06450b93a6390e2c758 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1303798346" 
	@${RM} ${OBJECTDIR}/_ext/1303798346/plib_uart1.o.d 
	@${RM} ${OBJECTDIR}/_ext/1303798346/plib_uart1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1303798346/plib_uart1.o.d" -o ${OBJECTDIR}/_ext/1303798346/plib_uart1.o ../src/config/pic32mz_w1_curiosity/peripheral/uart/plib_uart1.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1903958431/app_sst26vf.o: ../src/config/pic32mz_w1_curiosity/sst26/app_sst26vf.c  .generated_files/flags/pic32mz_w1_curiosity/e45d54a0f3abd716c04a32ba1fb45d3faf47f823 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1903958431" 
	@${RM} ${OBJECTDIR}/_ext/1903958431/app_sst26vf.o.d 
	@${RM} ${OBJECTDIR}/_ext/1903958431/app_sst26vf.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1903958431/app_sst26vf.o.d" -o ${OBJECTDIR}/_ext/1903958431/app_sst26vf.o ../src/config/pic32mz_w1_curiosity/sst26/app_sst26vf.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1903958431/ext_flash.o: ../src/config/pic32mz_w1_curiosity/sst26/ext_flash.c  .generated_files/flags/pic32mz_w1_curiosity/eac3ede6e098bf8fcba5d754326e725937769f15 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1903958431" 
	@${RM} ${OBJECTDIR}/_ext/1903958431/ext_flash.o.d 
	@${RM} ${OBJECTDIR}/_ext/1903958431/ext_flash.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1903958431/ext_flash.o.d" -o ${OBJECTDIR}/_ext/1903958431/ext_flash.o ../src/config/pic32mz_w1_curiosity/sst26/ext_flash.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1737632808/initialization.o: ../src/config/pic32mz_w1_curiosity/initialization.c  .generated_files/flags/pic32mz_w1_curiosity/6392c57d34c76792d4849b9375101cdceb1a610e .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1737632808" 
	@${RM} ${OBJECTDIR}/_ext/1737632808/initialization.o.d 
	@${RM} ${OBJECTDIR}/_ext/1737632808/initialization.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1737632808/initialization.o.d" -o ${OBJECTDIR}/_ext/1737632808/initialization.o ../src/config/pic32mz_w1_curiosity/initialization.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1737632808/pmu_init.o: ../src/config/pic32mz_w1_curiosity/pmu_init.c  .generated_files/flags/pic32mz_w1_curiosity/30b5def58c63a5142441c5f4d70342f0b6f892c5 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1737632808" 
	@${RM} ${OBJECTDIR}/_ext/1737632808/pmu_init.o.d 
	@${RM} ${OBJECTDIR}/_ext/1737632808/pmu_init.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1737632808/pmu_init.o.d" -o ${OBJECTDIR}/_ext/1737632808/pmu_init.o ../src/config/pic32mz_w1_curiosity/pmu_init.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1737632808/interrupts.o: ../src/config/pic32mz_w1_curiosity/interrupts.c  .generated_files/flags/pic32mz_w1_curiosity/f92f6ae8ccea46e77ea5da73e73a9daa1d36aff1 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1737632808" 
	@${RM} ${OBJECTDIR}/_ext/1737632808/interrupts.o.d 
	@${RM} ${OBJECTDIR}/_ext/1737632808/interrupts.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1737632808/interrupts.o.d" -o ${OBJECTDIR}/_ext/1737632808/interrupts.o ../src/config/pic32mz_w1_curiosity/interrupts.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1737632808/exceptions.o: ../src/config/pic32mz_w1_curiosity/exceptions.c  .generated_files/flags/pic32mz_w1_curiosity/a0589b60f1b614e576819558f5d64e6f6c5d2fb5 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1737632808" 
	@${RM} ${OBJECTDIR}/_ext/1737632808/exceptions.o.d 
	@${RM} ${OBJECTDIR}/_ext/1737632808/exceptions.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1737632808/exceptions.o.d" -o ${OBJECTDIR}/_ext/1737632808/exceptions.o ../src/config/pic32mz_w1_curiosity/exceptions.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1360937237/main.o: ../src/main.c  .generated_files/flags/pic32mz_w1_curiosity/76f7d0735d036ab84ea5bd00c64ac0652cfe5f9b .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/main.o.d" -o ${OBJECTDIR}/_ext/1360937237/main.o ../src/main.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
else
${OBJECTDIR}/_ext/1611361252/bootloader_uart.o: ../src/config/pic32mz_w1_curiosity/bootloader/bootloader_uart.c  .generated_files/flags/pic32mz_w1_curiosity/ec40d83068588dcafabf60cb15501f7e0b12d182 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1611361252" 
	@${RM} ${OBJECTDIR}/_ext/1611361252/bootloader_uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/1611361252/bootloader_uart.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1611361252/bootloader_uart.o.d" -o ${OBJECTDIR}/_ext/1611361252/bootloader_uart.o ../src/config/pic32mz_w1_curiosity/bootloader/bootloader_uart.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1611361252/bootloader_common.o: ../src/config/pic32mz_w1_curiosity/bootloader/bootloader_common.c  .generated_files/flags/pic32mz_w1_curiosity/eb66a480ea0844b9567063b0629c2ced95a2915e .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1611361252" 
	@${RM} ${OBJECTDIR}/_ext/1611361252/bootloader_common.o.d 
	@${RM} ${OBJECTDIR}/_ext/1611361252/bootloader_common.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1611361252/bootloader_common.o.d" -o ${OBJECTDIR}/_ext/1611361252/bootloader_common.o ../src/config/pic32mz_w1_curiosity/bootloader/bootloader_common.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1481979610/plib_clk.o: ../src/config/pic32mz_w1_curiosity/peripheral/clk/plib_clk.c  .generated_files/flags/pic32mz_w1_curiosity/854fe79fb7296395b830f7efe043fc97f9caa55c .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1481979610" 
	@${RM} ${OBJECTDIR}/_ext/1481979610/plib_clk.o.d 
	@${RM} ${OBJECTDIR}/_ext/1481979610/plib_clk.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1481979610/plib_clk.o.d" -o ${OBJECTDIR}/_ext/1481979610/plib_clk.o ../src/config/pic32mz_w1_curiosity/peripheral/clk/plib_clk.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1145473014/plib_coretimer.o: ../src/config/pic32mz_w1_curiosity/peripheral/coretimer/plib_coretimer.c  .generated_files/flags/pic32mz_w1_curiosity/7f41c7a35a652c9f2a677d2bff002aebdb6d06ca .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1145473014" 
	@${RM} ${OBJECTDIR}/_ext/1145473014/plib_coretimer.o.d 
	@${RM} ${OBJECTDIR}/_ext/1145473014/plib_coretimer.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1145473014/plib_coretimer.o.d" -o ${OBJECTDIR}/_ext/1145473014/plib_coretimer.o ../src/config/pic32mz_w1_curiosity/peripheral/coretimer/plib_coretimer.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1303341575/plib_evic.o: ../src/config/pic32mz_w1_curiosity/peripheral/evic/plib_evic.c  .generated_files/flags/pic32mz_w1_curiosity/cd955f779a0eaab4a8dd0af94131761900d988aa .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1303341575" 
	@${RM} ${OBJECTDIR}/_ext/1303341575/plib_evic.o.d 
	@${RM} ${OBJECTDIR}/_ext/1303341575/plib_evic.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1303341575/plib_evic.o.d" -o ${OBJECTDIR}/_ext/1303341575/plib_evic.o ../src/config/pic32mz_w1_curiosity/peripheral/evic/plib_evic.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1303395403/plib_gpio.o: ../src/config/pic32mz_w1_curiosity/peripheral/gpio/plib_gpio.c  .generated_files/flags/pic32mz_w1_curiosity/75329c16a6823ef853b7ffa39d6665f9ef93b55c .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1303395403" 
	@${RM} ${OBJECTDIR}/_ext/1303395403/plib_gpio.o.d 
	@${RM} ${OBJECTDIR}/_ext/1303395403/plib_gpio.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1303395403/plib_gpio.o.d" -o ${OBJECTDIR}/_ext/1303395403/plib_gpio.o ../src/config/pic32mz_w1_curiosity/peripheral/gpio/plib_gpio.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1481968727/plib_nvm.o: ../src/config/pic32mz_w1_curiosity/peripheral/nvm/plib_nvm.c  .generated_files/flags/pic32mz_w1_curiosity/8ee77d6db2bf5d9fc57fa4f5494c3dcd992c5cc .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1481968727" 
	@${RM} ${OBJECTDIR}/_ext/1481968727/plib_nvm.o.d 
	@${RM} ${OBJECTDIR}/_ext/1481968727/plib_nvm.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1481968727/plib_nvm.o.d" -o ${OBJECTDIR}/_ext/1481968727/plib_nvm.o ../src/config/pic32mz_w1_curiosity/peripheral/nvm/plib_nvm.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/521489940/plib_spi1_master.o: ../src/config/pic32mz_w1_curiosity/peripheral/spi/spi_master/plib_spi1_master.c  .generated_files/flags/pic32mz_w1_curiosity/c1d9fc8ed45392f06c8a3178cf7ac829cfe43f7e .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/521489940" 
	@${RM} ${OBJECTDIR}/_ext/521489940/plib_spi1_master.o.d 
	@${RM} ${OBJECTDIR}/_ext/521489940/plib_spi1_master.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/521489940/plib_spi1_master.o.d" -o ${OBJECTDIR}/_ext/521489940/plib_spi1_master.o ../src/config/pic32mz_w1_curiosity/peripheral/spi/spi_master/plib_spi1_master.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1303798346/plib_uart1.o: ../src/config/pic32mz_w1_curiosity/peripheral/uart/plib_uart1.c  .generated_files/flags/pic32mz_w1_curiosity/f705514954a848b581ed11e864769ca6220bc3c2 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1303798346" 
	@${RM} ${OBJECTDIR}/_ext/1303798346/plib_uart1.o.d 
	@${RM} ${OBJECTDIR}/_ext/1303798346/plib_uart1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1303798346/plib_uart1.o.d" -o ${OBJECTDIR}/_ext/1303798346/plib_uart1.o ../src/config/pic32mz_w1_curiosity/peripheral/uart/plib_uart1.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1903958431/app_sst26vf.o: ../src/config/pic32mz_w1_curiosity/sst26/app_sst26vf.c  .generated_files/flags/pic32mz_w1_curiosity/c8c27c4962a4dd386d4c2ef9d5bb9eef8e23c7f5 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1903958431" 
	@${RM} ${OBJECTDIR}/_ext/1903958431/app_sst26vf.o.d 
	@${RM} ${OBJECTDIR}/_ext/1903958431/app_sst26vf.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1903958431/app_sst26vf.o.d" -o ${OBJECTDIR}/_ext/1903958431/app_sst26vf.o ../src/config/pic32mz_w1_curiosity/sst26/app_sst26vf.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1903958431/ext_flash.o: ../src/config/pic32mz_w1_curiosity/sst26/ext_flash.c  .generated_files/flags/pic32mz_w1_curiosity/5f1f31f6716cd1416c1d17453fb8647b32bdbbeb .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1903958431" 
	@${RM} ${OBJECTDIR}/_ext/1903958431/ext_flash.o.d 
	@${RM} ${OBJECTDIR}/_ext/1903958431/ext_flash.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1903958431/ext_flash.o.d" -o ${OBJECTDIR}/_ext/1903958431/ext_flash.o ../src/config/pic32mz_w1_curiosity/sst26/ext_flash.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1737632808/initialization.o: ../src/config/pic32mz_w1_curiosity/initialization.c  .generated_files/flags/pic32mz_w1_curiosity/3377a14921c20a8ac81e6091cec002e2cbf850cd .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1737632808" 
	@${RM} ${OBJECTDIR}/_ext/1737632808/initialization.o.d 
	@${RM} ${OBJECTDIR}/_ext/1737632808/initialization.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1737632808/initialization.o.d" -o ${OBJECTDIR}/_ext/1737632808/initialization.o ../src/config/pic32mz_w1_curiosity/initialization.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1737632808/pmu_init.o: ../src/config/pic32mz_w1_curiosity/pmu_init.c  .generated_files/flags/pic32mz_w1_curiosity/32b4e7244a546e219f90b274bd77dfb641343909 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1737632808" 
	@${RM} ${OBJECTDIR}/_ext/1737632808/pmu_init.o.d 
	@${RM} ${OBJECTDIR}/_ext/1737632808/pmu_init.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1737632808/pmu_init.o.d" -o ${OBJECTDIR}/_ext/1737632808/pmu_init.o ../src/config/pic32mz_w1_curiosity/pmu_init.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1737632808/interrupts.o: ../src/config/pic32mz_w1_curiosity/interrupts.c  .generated_files/flags/pic32mz_w1_curiosity/56a64caaddab753eb8237860bdf342aff27d3ac7 .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1737632808" 
	@${RM} ${OBJECTDIR}/_ext/1737632808/interrupts.o.d 
	@${RM} ${OBJECTDIR}/_ext/1737632808/interrupts.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1737632808/interrupts.o.d" -o ${OBJECTDIR}/_ext/1737632808/interrupts.o ../src/config/pic32mz_w1_curiosity/interrupts.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1737632808/exceptions.o: ../src/config/pic32mz_w1_curiosity/exceptions.c  .generated_files/flags/pic32mz_w1_curiosity/1417dd1351e282125559068675fb33dd3ac5d50f .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1737632808" 
	@${RM} ${OBJECTDIR}/_ext/1737632808/exceptions.o.d 
	@${RM} ${OBJECTDIR}/_ext/1737632808/exceptions.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1737632808/exceptions.o.d" -o ${OBJECTDIR}/_ext/1737632808/exceptions.o ../src/config/pic32mz_w1_curiosity/exceptions.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1360937237/main.o: ../src/main.c  .generated_files/flags/pic32mz_w1_curiosity/f3e71d5d686b254a8eef979a994653423ac0a09b .generated_files/flags/pic32mz_w1_curiosity/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O2 -fno-common -I"../src" -I"../src/config/pic32mz_w1_curiosity" -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/main.o.d" -o ${OBJECTDIR}/_ext/1360937237/main.o ../src/main.c    -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/pic32mz_w1_curiosity.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    ../src/config/pic32mz_w1_curiosity/btl.ld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -g   -mprocessor=$(MP_PROCESSOR_OPTION)  -o ${DISTDIR}/pic32mz_w1_curiosity.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)      -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D=__DEBUG_D,--defsym=_min_heap_size=512,--gc-sections,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,${DISTDIR}/memoryfile.xml -mdfp="${DFP_DIR}"
	
else
${DISTDIR}/pic32mz_w1_curiosity.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   ../src/config/pic32mz_w1_curiosity/btl.ld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o ${DISTDIR}/pic32mz_w1_curiosity.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_pic32mz_w1_curiosity=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_heap_size=512,--gc-sections,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,${DISTDIR}/memoryfile.xml -mdfp="${DFP_DIR}"
	${MP_CC_DIR}\\xc32-bin2hex ${DISTDIR}/pic32mz_w1_curiosity.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(wildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
