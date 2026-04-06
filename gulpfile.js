import childProcess from 'child_process';
import { promises as fsp } from 'fs';
import os from 'os';
import path from 'path';
import { fileURLToPath } from 'url';

const ZML_VERSION = '9.0.0';

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

export const update_support_package_version = async () => {
  const makefilePath = 'zaberMotionSupport/Makefile';
  const contents = await fsp.readFile(makefilePath, 'utf8');
  const updated = contents.replace(/^(ZML_VERSION\s*=\s*).*$/m, `$1${ZML_VERSION}`);
  await fsp.writeFile(makefilePath, updated);
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
  await update_release_for_module(`${EPICS_SUPPORT}/autosave`);
  await update_release_for_module(`${EPICS_SUPPORT}/busy`);
  await update_release_for_module(`${EPICS_SUPPORT}/motor`);

  // asyn-specific changes
  const asynReleasePath = `${EPICS_SUPPORT}/asyn/configure/RELEASE`;
  const asynReleaseContents = await fsp.readFile(asynReleasePath, 'utf8');
  await fsp.writeFile(asynReleasePath, asynReleaseContents
    .replace(/^#?\s*SNCSEQ\s*=.*/m, `SNCSEQ=${EPICS_SUPPORT}/sequencer`));
  if (process.platform === 'linux') {
    const asynPath = `${EPICS_SUPPORT}/asyn/configure/CONFIG_SITE`;
    console.log('updating file with asyn path: ', asynPath);
    const asynContents = await fsp.readFile(asynPath, 'utf8');
    const newContents = asynContents.replace(/^#? *TIRPC *=.*/m, `TIRPC = YES`);
    await fsp.writeFile(asynPath, newContents);
  }

  // busy-specific changes
  const busyReleasePath = `${EPICS_SUPPORT}/busy/configure/RELEASE`;
  const busyReleaseContents = await fsp.readFile(busyReleasePath, 'utf8');
  await fsp.writeFile(busyReleasePath, busyReleaseContents
    .replace(/^#?\s*ASYN\s*=.*/m, `ASYN=${EPICS_SUPPORT}/asyn`)
    .replace(/^#?\s*BUSY\s*=.*/m, `BUSY=${EPICS_SUPPORT}/busy`)
    .replace(/^#?\s*AUTOSAVE\s*=.*/m, `AUTOSAVE=${EPICS_SUPPORT}/autosave`));

  // motor-specific changes
  const motorReleasePath = `${EPICS_SUPPORT}/motor/configure/RELEASE`;
  const motorReleaseContents = await fsp.readFile(motorReleasePath, 'utf8');
  const newMotorReleaseContents = motorReleaseContents
    .replace(/^#?\s*ASYN\s*=.*/m, `ASYN=${EPICS_SUPPORT}/asyn`)
    .replace(/^#?\s*BUSY\s*=.*/m, `BUSY=${EPICS_SUPPORT}/busy`)
    .replace(/^#?\s*SNCSEQ\s*=.*/m, `SNCSEQ=${EPICS_SUPPORT}/sequencer`)
    .replace(/^#?\s*IPAC\s*=(.*)/m, '#IPAC=$1');
  await fsp.writeFile(motorReleasePath, newMotorReleaseContents);

  const modulePath = `${EPICS_SUPPORT}/motor/modules/Makefile`;
  const moduleContents = await fsp.readFile(modulePath, 'utf8');
  const newModuleContents = moduleContents
    .replaceAll(/^SUBMODULES\s*\+=(.*)/gm, '#SUBMODULES=$1')
    .replace(/^#SUBMODULES.*/m, 'SUBMODULES += motorZaberMotion');
  await fsp.writeFile(modulePath, newModuleContents);

  // changes specific to this module
  const motorZaberMotionPath = path.dirname(fileURLToPath(import.meta.url));
  const configSitePath = path.join(motorZaberMotionPath, 'configure', 'CONFIG_SITE.local');
  await fsp.writeFile(configSitePath, `BUILD_IOCS=YES${os.EOL}`);
}

export const build = async () => {
  await update_support_configs();

  // run make in all support modules
  await exec(`make -C ${EPICS_SUPPORT}/sequencer`);
  await exec(`make -C ${EPICS_SUPPORT}/asyn`);
  await exec(`make -C ${EPICS_SUPPORT}/autosave`);
  await exec(`make -C ${EPICS_SUPPORT}/busy`);
  await build_motor();
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
