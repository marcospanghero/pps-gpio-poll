/*
 * pps-gpio-poll.c -- PPS client driver using GPIO (polling mode)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define pr_fmt(fmt) "pps-gpio-poll" ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pps_kernel.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

/* #define GPIO_ECHO */

static int gpio;

static int capture = 1;
static int poll = 100;
static int wait = 15;
static int iter = 2000;
static int rate = 1;
static int interval;
static int debug;

module_param(gpio, int, S_IRUSR);
MODULE_PARM_DESC(gpio, "PPS GPIO");

module_param(capture, int, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(capture, "capture PPS event: 0 = falling, 1 = rising");
module_param(poll, int, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(poll, "polling interval (microseconds)");
module_param(wait, int, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(wait, "time to wait for PPS event (microseconds)");
module_param(iter, int, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(rate, "PPS rate (Hz)");
module_param(rate, int, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(iter, "maximum number of GPIO reads in the wait loop");
module_param(debug, int, S_IRUSR | S_IWUSR);


static struct pps_device *pps;
static ktime_t last_ts;
static struct hrtimer timer;
static int gpio_value;

static int pps_gpio_register(void)
{

	int i, ret, pps_default_params;
	ktime_t ts1, ts2;
	struct pps_source_info info = {
		.name	= KBUILD_MODNAME,
		.path	= "",
		.mode	= PPS_CAPTUREASSERT | PPS_OFFSETASSERT | \
				  PPS_CAPTURECLEAR | PPS_OFFSETCLEAR | \
				  PPS_CANWAIT | PPS_TSFMT_TSPEC,
		.owner	= THIS_MODULE,
		.dev	= NULL
	};

	/* GPIO setup */
	ret = gpio_request(gpio, "PPS");
	if (ret) {
		pr_warn("failed to request PPS GPIO %u\n", gpio);
		return -EINVAL;
	}
	ret = gpio_direction_input(gpio);
	if (ret) {
	    pr_warn("failed to set PPS GPIO direction\n");
        gpio_free(gpio);
        return -EINVAL;
	}

	/* register PPS source */
	pps_default_params = PPS_CAPTUREASSERT | PPS_OFFSETASSERT;
	pps = pps_register_source(&info, pps_default_params);
	if (pps == NULL) {
		pr_err("failed to register GPIO PPS source\n");
	}

	ts1 = ktime_get();
	gpio_get_value(gpio);

	ts1 = ktime_get();

	for (i = 0; i < 100; i++){
		gpio_get_value(gpio);
	}
	ts2 = ktime_get();

	pr_info("Registered GPIO %d as PPS source (precision %d ns)\n",
		gpio, (int)(ktime_to_ns(ts2) - ktime_to_ns(ts1)) / 100);
	return 0;

}

static int pps_gpio_remove(void)
{
	gpio_free(gpio);
	pps_unregister_source(pps);
	pr_info("Removed GPIO %d as PPS source\n", gpio);
	return 0;
}

static enum hrtimer_restart gpio_wait(struct hrtimer *t);

static enum hrtimer_restart gpio_poll(struct hrtimer *t)
{
	ktime_t ktime = ktime_set(0, poll*1000);

	int value = gpio_get_value(gpio);
	if (value != gpio_value) {
		if (value == capture) {
			/* got a PPS event, start busy waiting 1.5*poll microseconds
			 * before the next event */
			ktime = ktime_set(0, (interval - poll - poll/2)*1000);
			timer.function = &gpio_wait;
			last_ts = ktime_set(0, 0);
		}
		gpio_value = value;
	}

	hrtimer_start(&timer, ktime, HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}

static enum hrtimer_restart gpio_wait(struct hrtimer *t)
{
	ktime_t ktime;
	struct pps_event_time ts;
	ktime_t monotonic;
	int i, have_ts = 0;
	unsigned long flags;

	/* read the GPIO value until the PPS event happens */
	local_irq_save(flags);
	pps_get_ts(&ts);
	for (i = 0; likely(i < iter && gpio_get_value(gpio) != capture); i++) {
	}

	pps_get_ts(&ts);
	monotonic = ktime_get();
	local_irq_restore(flags);

	if (likely(i > 0 && i < iter)) {
		have_ts = 1;
		if (unlikely(ktime_to_ns(last_ts) == 0)) {
			/* this is the first event, start busy waiting
			 * poll/2 microseconds before the next */
			ktime = ktime_set(0, (interval - poll/2)*1000);
		} else {
			/* we know the time between events, start busy waiting
			 * "wait" microseconds before the next event */
			ktime = monotonic;
			ktime = ktime_sub(ktime, last_ts);
			ktime = ktime_sub_ns(ktime, wait*1000);
		}
		last_ts = monotonic;
	} else {
		/* we missed the event inside the loop, return to polling mode */
		pr_info("missed PPS pulse\n");
		ktime = ktime_set(0, poll*1000);
		timer.function = &gpio_poll;
	}
	hrtimer_start(&timer, ktime, HRTIMER_MODE_REL);

	if (have_ts) {
		pps_event(pps, &ts, capture ? PPS_CAPTUREASSERT : PPS_CAPTURECLEAR, NULL);
	}
	if (debug)
		pr_info("%d GPIO reads\n", i);

	return HRTIMER_NORESTART;
}


static int __init pps_gpio_init(void)
{
	int ret;
	ktime_t ktime;

	ret = pps_gpio_register();
	if (ret < 0)
		return ret;

	gpio_value = gpio_get_value(gpio);
	interval = 1000000 / rate;

	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	timer.function = &gpio_poll;
	ktime = ktime_set(0, 1000000);
	hrtimer_start(&timer, ktime, HRTIMER_MODE_REL);


	return 0;
}

static void __exit pps_gpio_exit(void)
{
	hrtimer_cancel(&timer);
	pps_gpio_remove();
}


module_init(pps_gpio_init);
module_exit(pps_gpio_exit);

MODULE_AUTHOR("Gabriel Ricalde");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");
