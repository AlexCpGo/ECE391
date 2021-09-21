### P2 Solution

1.  MTCP_BIOC_ON: Enables button “interrupt on change”-whenever there is a state change in any of the port pin. As a result, the controller will jump to the interrupt vector when the state of the button is changed.

    When to sent: when started and controller’s buttons need to be used

    Effect: gives controller an interrupt whenever the button status is pressed

    Return: MTCP_ACK

	MTCP_LED_SET: LED display values

	When to sent: Need to display values on LED

	Effect: When LED display is in user mode, it will display value specified by this command

	Return: MTCP_ACK

2.  MTCP_ACK: signal for successful cmd, received when each command is finished

    MTCP_BIOC_EVENT: to inform controller which button is pressed

	MTCP_RESET: to generate when RESET button is pressed or on an MTCP_RESET_DEV command

3.  Because tuxctl_idisc_data_callback is called by interrupt context and tuxctl_handle_packet is called by tuxctl_idisc_data_callback, if the code waited in the interrupt, it would cause the problem of dead lock.
