#!/bin/bash

SCRIPTNAME="rundbbuild.sh"
RECONFIG=FALSE
DEVBUILD=FALSE
CIBUILD=FALSE
DB820C=FALSE
LUMIA950XL=FALSE

function HelpMsg()
{
  echo "Usage: $SCRIPTNAME [Options]"
  echo
  echo "Configure EDK2 build environment, then kicks off build for Dragonboard 410c."
  echo
  echo
  echo "Options: "
  echo "  --help, -h, -?        Print this help screen and exit."
  echo
  echo "  --development, -dev   Run development build (dirty)."
  echo
  echo "  --820c, -820c         Run build for Dragonboard 820c."
  echo
  echo "  --950xl, -950xl       Run build for Lumia 950 XL."
  echo
  echo "  --production, -ci     Run CI build (clean)."
  echo
}

function SetWorkspace()
{
  #
  # If WORKSPACE is already set, then we can return right now
  #
  if [ -n "$WORKSPACE" ]
  then
    return 0
  fi

  #
  # Check for BaseTools/BuildEnv before dirtying the user's environment.
  #
  if [ ! -f BaseTools/BuildEnv ] && [ -z "$EDK_TOOLS_PATH" ]
  then
    echo BaseTools not found in your tree, and EDK_TOOLS_PATH is not set.
    echo Please point EDK_TOOLS_PATH at the directory that contains
    echo the EDK2 BuildEnv script.
    return 1
  fi

  #
  # Set $WORKSPACE
  #
  export WORKSPACE=`pwd`

  return 0
}

function SetupEnv()
{
  if [ -n "$EDK_TOOLS_PATH" ]
  then
    . $EDK_TOOLS_PATH/BuildEnv
  elif [ -f "$WORKSPACE/BaseTools/BuildEnv" ]
  then
    . $WORKSPACE/BaseTools/BuildEnv
  elif [ -n "$PACKAGES_PATH" ]
  then 
    PATH_LIST=$PACKAGES_PATH
    PATH_LIST=${PATH_LIST//:/ }
    for DIR in $PATH_LIST
    do
      if [ -f "$DIR/BaseTools/BuildEnv" ]
      then
        export EDK_TOOLS_PATH=$DIR/BaseTools
        . $DIR/BaseTools/BuildEnv
        break
      fi
    done
  else
    echo BaseTools not found in your tree, and EDK_TOOLS_PATH is not set.
    echo Please check that WORKSPACE or PACKAGES_PATH is not set incorrectly
    echo in your shell, or point EDK_TOOLS_PATH at the directory that contains
    echo the EDK2 BuildEnv script.
    return 1
  fi
}

function FixPermission()
{
  if [ -d "Lumia950XLPkg" ]; then
   chmod +x Lumia950XLPkg/Tools/*.ps1
   chmod +x Lumia950XLPkg/Tools/PsModules/*.psm1
  fi
}

function SourceEnv()
{
  SetWorkspace &&
  SetupEnv &&
  FixPermission
}

function DevelopmentBuild()
{
  if [ "$DB820C" = TRUE ]; then
    ./Dragonboard820cPkg/Tools/edk2-build.ps1
  elif [ "$LUMIA950XL" = TRUE ]; then
    ./Lumia950XLPkg/Tools/edk2-build.ps1
  else
    ./DragonboardPkg/Tools/edk2-build.ps1
  fi

  if [ ! $? -eq 0 ]; then
      echo "[Builder] Build failed."
      return $?
  fi
  
}

function CIBuild()
{
  if [ "$DB820C" = TRUE ]; then
    ./Dragonboard820cPkg/Tools/edk2-build.ps1 -Clean
  elif [ "$LUMIA950XL" = TRUE ]; then
    ./Lumia950XLPkg/Tools/edk2-build.ps1 -Clean
  else
    ./DragonboardPkg/Tools/edk2-build.ps1 -Clean
  fi

  if [ ! $? -eq 0 ]; then
      echo "[Builder] Build failed."
      return $?
  fi
  
}

I=$#
while [ $I -gt 0 ]
do
  case "$1" in
    --development|-dev)
      # Development build
      echo "[Builder] Configure environment and run development build."
      if [ "$CIBUILD" = TRUE ]; then
        echo "[Builder] Only one build configuration can be selected."
        return 1
      else
        DEVBUILD=TRUE
      fi
      shift
    ;;
    --production|-ci)
      # CI Build
      echo "[Builder] Configure environment and run CI build (clean)."
      if [ "$DEVBUILD" = TRUE ]; then
        echo "[Builder] Only one build configuration can be selected."
        return 1
      else
        CIBUILD=TRUE
      fi
      shift
    ;;
    --820c|-820c)
      # 820C build
      echo "[Builder] Run Dragonboard 820c Build."
      DB820C=TRUE
      shift
    ;;
    --950xl|-950xl)
      # 950XL build
      echo "[Builder] Run Lumia 950 / Lumia 950 XL Build."
      LUMIA950XL=TRUE
      shift
    ;;
    -?|-h|--help|*)
      HelpMsg
      break
    ;;
  esac
  I=$(($I - 1))
done

if [ $I -gt 0 ]
then
  return 1
else

  echo "[Builder] Configure environment."
  SourceEnv
  if [ ! $? -eq 0 ]; then
      echo "Unable to configure EDK2 environment."
      return $?
  fi

  # Run build
  if [ "$DEVBUILD" = TRUE ]; then
    DevelopmentBuild
  elif [ "$CIBUILD" = TRUE ]; then
    CIBuild
  else
    echo "[Builder] You must specify one build option."
    return 1
  fi

  if [ ! $? -eq 0 ]; then
      echo "[Builder] Build failed."
      return $?
  fi
fi
