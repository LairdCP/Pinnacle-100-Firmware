# Memfault SDK Integration

The [AWS Out-of-Box (OOB) Demo](readme_ltem_aws.md) (Pinnacle 100 DVK and MG100) integrates the [Memfault](https://docs.memfault.com/docs/mcu/introduction/) SDK.
Memfault allows the user to monitor devices remotely debug firmware and more. See [memfault.com](https://memfault.com) for more info.

The Memfault SDK is integrated in the OOB firmware version 4.x and newer.
The [pre-built OOB demo binaries](https://github.com/LairdCP/Pinnacle-100-Firmware/releases) are configured to communicate to the Laird Connectivity Memfault project instance.

## OOB Demo

The OOB demo uses Memfault to track various firmware statistics and issues.

In the OOB demo, Memfault is used to track:

- Reboots and reasons for reboot
- Coredumps caused by crashes
- Various LTE statistics (signal strength, time to fix, number of dropped connections)

Once an LTE connection is obtained, Memfault data is pushed over HTTPS.

After connecting to AWS, Memfault data is sent over MQTT to a topic that has the format:

```
prod/<board>/<device_id>/memfault/<memfault_project_key>/chunk
```

The data sent to this topic is raw binary chunk data generated by the Memfault SDK.
Data is only sent if it is available and at a period of once per hour.

## Sign Up for an Account

Visit [https://goto.memfault.com/create-key/pinnacle-100](https://goto.memfault.com/create-key/pinnacle-100)
to sign up for a FREE Memfault account if you don't already have one!

## Building the OOB Demo

When building the OOB demo with Memfault integration turned on, a Memfault project key needs to be provided.
The project key is needed so the device knows what Memfault project to send the data to.

> **Note:** If the following project options are not configured, the firmware will fail to build.

Set `CONFIG_MEMFAULT_NCS_PROJECT_KEY="my_key"` in your project config to the project key for your account.

Along with setting the Memfault project key. `CONFIG_MEMFAULT_NCS_FW_VERSION_PREFIX` and `CONFIG_MEMFAULT_NCS_FW_TYPE` need to be set in your project config.
See [here](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/include/memfault_ncs.html?highlight=memfault#configuration-options-in-ncs) for more details.

If you wish not to use the Memfault integration when building your app, set `CONFIG_LCZ_MEMFAULT=n` in your project config.
