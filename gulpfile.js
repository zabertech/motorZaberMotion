import childProcess from 'child_process';
import { promises as fsp } from 'fs';

const ZML_VERSION = '7.2.0';

const EPICS_BASE = process.env.EPICS_BASE;
const EPICS_SUPPORT = `${process.env.HOME}/EPICS/support`;

const exec = (command) => new Promise((resolve, reject) => {
  const child = childProcess.exec(command, undefined, err => err ? reject(err) : resolve());
  child.stdout?.pipe(process.stdout);
  child.stderr?.pipe(process.stderr);
});

export const update_support_package = async () => {
  const localDestination = `zaberMotionSupport/ZaberMotionCppSupport-${ZML_VERSION}.zip`;

  await fsp.rm('zaberMotionSupport/ZaberMotionCppSupport', { recursive: true }).catch(() => false);
  await exec(`aws s3 cp s3://026596715358-us-west-2-production--downloads/downloads/ZML/CPP/${ZML_VERSION}/ZaberMotionCppSupport-${ZML_VERSION}.zip ${localDestination}`);
  await exec(`unzip ${localDestination} -d zaberMotionSupport`);
  await fsp.rm(localDestination);
}

const update_release_for_module = async modulePath => {
  const releasePath = `${modulePath}/configure/RELEASE`;
  const releaseContents = await fsp.readFile(releasePath, 'utf8');
  const newContents = releaseContents
    .replace(/^#? *SUPPORT *=.*$/m, `SUPPORT = ${EPICS_SUPPORT}`)
    .replace(/^#? *EPICS_BASE *=.*$/m, `EPICS_BASE = ${EPICS_BASE}`);
  await fsp.writeFile(releasePath, newContents);
}

export const update_epics_configs = async () => {
  await update_release_for_module(`${EPICS_SUPPORT}/asyn`);
  await update_release_for_module(`${EPICS_SUPPORT}/sequencer`);
  await update_release_for_module(`${EPICS_SUPPORT}/motor`);

  // asyn-specific changes for linux
  if (process.platform === 'linux') {
    const asynPath = '/Users/colby.sparks/EPICS/support/asyn/configure/CONFIG_SITE';
    console.log('updating file with asyn path: ', asynPath);
    const asynContents = await fsp.readFile(asynPath, 'utf8');
    const newContents = asynContents.replace(/^#? *TIRPC *=.*/m, `TIRPC = YES`);
    await fsp.writeFile(asynPath, newContents);
  }

  // motor-specific changes
  const releasePath = `${EPICS_SUPPORT}/motor/configure/RELEASE`;
  const releaseContents = await fsp.readFile(releasePath, 'utf8');
  const newContents = releaseContents
    .replace(/^#? *ASYN *=.*/m, `ASYN=${EPICS_SUPPORT}/asyn`)
    .replace(/^#? *SNCSEQ *=.*/m, `SNCSEQ=${EPICS_SUPPORT}/sequencer`);
  await fsp.writeFile(releasePath, newContents);

  // run make in all support modules
  await exec(`make -C ${EPICS_SUPPORT}/asyn`);
  await exec(`make -C ${EPICS_SUPPORT}/sequencer`);
  await exec(`make -C ${EPICS_SUPPORT}/motor`);
}

