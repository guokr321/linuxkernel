/*
 *	Guoke 
 */

#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/crc32.h>
#include <linux/mii.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <asm/delay.h>
#include <asm/irq.h>
#include <asm/io.h>

#include "dm9000.h"

/* Board/System/Debug information/definition ---------------- */
#define CARDNAME	"vnet"
#define DRV_VERSION	"0.01"

/*
 * 发送超时默认5s
 */
static int watchdog = 5000;
module_param(watchdog, int, 0400);
MODULE_PARM_DESC(watchdog, "transmit timeout in milliseconds");

/*
 * 调试开关
 */
static int debug;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "dm9000 debug level (0-6)");;

/* debug code */
#define vnet_dbg(db, lev, msg...) do {		\
	if ((lev) < debug) {				\
		dev_dbg(db->dev, msg);			\
	}						\
} while (0)


//要实现，这是硬件信息应该也不用实现
static const struct ethtool_ops dm9000_ethtool_ops = {
	.get_drvinfo		= dm9000_get_drvinfo,
	.get_msglevel		= dm9000_get_msglevel,
	.set_msglevel		= dm9000_set_msglevel,
	.nway_reset		= dm9000_nway_reset,
	.get_link		= dm9000_get_link,
	.get_wol		= dm9000_get_wol,
	.set_wol		= dm9000_set_wol,
	.get_eeprom_len		= dm9000_get_eeprom_len,
	.get_eeprom		= dm9000_get_eeprom,
	.set_eeprom		= dm9000_set_eeprom,
	.get_link_ksettings	= dm9000_get_link_ksettings,
	.set_link_ksettings	= dm9000_set_link_ksettings,
};

//这个是在probe时注册的
static const struct net_device_ops dm9000_netdev_ops = {
	.ndo_open		= dm9000_open,
	.ndo_stop		= dm9000_stop,
	.ndo_start_xmit		= dm9000_start_xmit,
	.ndo_tx_timeout		= dm9000_timeout,
	.ndo_set_rx_mode	= dm9000_hash_table,
	.ndo_do_ioctl		= dm9000_ioctl,
	.ndo_set_features	= dm9000_set_features,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address	= eth_mac_addr,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= dm9000_poll_controller,
#endif
};

static const struct dev_pm_ops dm9000_drv_pm_ops = {
	.suspend	= dm9000_drv_suspend,
	.resume		= dm9000_drv_resume,
};

static int
dm9000_drv_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id dm9000_of_matches[] = {
	{ .compatible = "davicom,dm9000", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, dm9000_of_matches);
#endif

static struct platform_driver dm9000_driver = {
	.driver	= {
		.name    = "dm9000",
		.pm	 = &dm9000_drv_pm_ops,  //这里面注册电源管理的callback
		.of_match_table = of_match_ptr(dm9000_of_matches),  //只是兼容性，不知道哪里用
	},
	.probe   = dm9000_probe,          //网卡加载时用    
	.remove  = dm9000_drv_remove,     //网卡卸载时用
};

module_platform_driver(dm9000_driver);

MODULE_AUTHOR("Sascha Hauer, Ben Dooks");
MODULE_DESCRIPTION("Davicom DM9000 network driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:dm9000");
