import childProcess from 'child_process';
import { time } from 'console';
import { promises as fsp } from 'fs';
import os from 'os';
import path from 'path';

const ZML_VERSION = '7.2.1';

const EPICS_BASE = process.env.EPICS_BASE;
const EPICS_SUPPORT = path.normalize(`${EPICS_BASE}/../support`);

const os_to_epics_arch = () => {
  const arch = os.arch();
  const platform = os.platform();
  if (arch === 'x64' && platform !== 'darwin') return 'x86_64';
  if (arch === 'x64' && platform === 'darwin') return 'x86';
  if (arch === 'arm64') return 'aarch64';
  if (arch === 'arm') return 'arm';
  return arch;
}

const exec = (command) => new Promise((resolve, reject) => {
  const child = childProcess.exec(command, undefined, err => err ? reject(err) : resolve());
  child.stdout?.pipe(process.stdout);
  child.stderr?.pipe(process.stderr);
});

export const update_support_package = async () => {
  const url = `https://www.zaber.com/support/software-downloads.php\\?product\\=zml_cpp_support\\&version\\=${ZML_VERSION}`;
  const localDestination = `zaberMotionSupport/ZaberMotionCppSupport-${ZML_VERSION}.zip`;

  await fsp.rm('zaberMotionSupport/ZaberMotionCppSupport', { recursive: true }).catch(() => false);
  await exec(`curl -L -o ${localDestination} ${url}`);
  await exec(`unzip ${localDestination} -d zaberMotionSupport`);
  await fsp.rm(localDestination);
}

const update_release_for_module = async modulePath => {
  const releasePath = `${modulePath}/configure/RELEASE`;
  const releaseContents = await fsp.readFile(releasePath, 'utf8');
  const newContents = releaseContents
    .replace(/^#?\s*SUPPORT\s*=.*$/m, `SUPPORT = ${EPICS_SUPPORT}`)
    .replace(/^#?\s*EPICS_BASE\s*=.*$/m, `EPICS_BASE = ${EPICS_BASE}`);
  await fsp.writeFile(releasePath, newContents);
}

const update_support_configs = async () => {
  await update_release_for_module(`${EPICS_SUPPORT}/asyn`);
  await update_release_for_module(`${EPICS_SUPPORT}/sequencer`);
  await update_release_for_module(`${EPICS_SUPPORT}/motor`);

  // asyn-specific changes for linux
  if (process.platform === 'linux') {
    const asynPath = `${EPICS_SUPPORT}/asyn/configure/CONFIG_SITE`;
    console.log('updating file with asyn path: ', asynPath);
    const asynContents = await fsp.readFile(asynPath, 'utf8');
    const newContents = asynContents.replace(/^#? *TIRPC *=.*/m, `TIRPC = YES`);
    await fsp.writeFile(asynPath, newContents);
  }

  // motor-specific changes
  const releasePath = `${EPICS_SUPPORT}/motor/configure/RELEASE`;
  const releaseContents = await fsp.readFile(releasePath, 'utf8');
  const newContents = releaseContents
    .replace(/^#?\s*ASYN\s*=.*/m, `ASYN=${EPICS_SUPPORT}/asyn`)
    .replace(/^#?\s*SNCSEQ\s*=.*/m, `SNCSEQ=${EPICS_SUPPORT}/sequencer`);
  await fsp.writeFile(releasePath, newContents);

  const modulePath = `${EPICS_SUPPORT}/motor/modules/Makefile`;
  const moduleContents = await fsp.readFile(modulePath, 'utf8');
  const newModuleContents = moduleContents
    .replaceAll(/^SUBMODULES\s*\+=(.*)/gm, '#SUBMODULES=$1')
    .replace(/^#SUBMODULES.*/m, 'SUBMODULES += motorZaberMotion');
  await fsp.writeFile(modulePath, newModuleContents);
}

export const build = async () => {
  await update_support_configs();

  // run make in all support modules
  await exec(`make -C ${EPICS_SUPPORT}/asyn`);
  await exec(`make -C ${EPICS_SUPPORT}/sequencer`);
  build_motor();
}

export const build_motor = async () => {
  await exec(`make -C ${EPICS_SUPPORT}/motor`);
}

export const clean_motor = async () => {
  await exec(`make realclean -C ${EPICS_SUPPORT}/motor`);
}

export const run_ioc = async () => {
  const iocBootFolder = `${EPICS_SUPPORT}/motor/modules/motorZaberMotion/iocs/zaberMotionIOC/iocBoot/iocZaberMotion`;
  await exec(`cd ${iocBootFolder} && ./st.cmd.${os.platform()}-${os_to_epics_arch()}`);
}

export const test = async () => {
  const zaberMotionTestPath = `${EPICS_SUPPORT}/motor/modules/motorZaberMotion/zaberMotionApp/test`;
  await fsp.rm(`${zaberMotionTestPath}/build`, { recursive: true }).catch(() => false);
  await fsp.mkdir(`${zaberMotionTestPath}/build`, { recursive: true });

  await exec(`
    cd ${zaberMotionTestPath}/build \
    && cmake ../ \
    && make -j${os.cpus().length} \
    && ctest --verbose
  `);
}
