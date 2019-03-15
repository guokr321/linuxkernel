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
#include <linux/dm9000.h>
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

struct board_info{
	
}


static const struct ethtool_ops dm9000_ethtool_ops = {
//	.get_drvinfo		
//	.get_msglevel		
//	.set_msglevel		
//	.nway_reset		
//	.get_link		
//	.get_wol		
//	.set_wol		
//	.get_eeprom_len		
//	.get_eeprom		
//	.set_eeprom		
//	.get_link_ksettings	
//	.set_link_ksettings	
};
static int open(struct net_device *dev)
{
	return 0;
}

static int stop(struct net_device *dev)
{
	return 0;
}

static netdev_tx_t start_xmit(struct sk_buff *skb，struct net_device *dev)
{
	return 0;
}

static void tx_timeout(struct net_device *dev)
{

}

static void set_rx_mode(struct net_device *dev)
{
	
}

static int do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	return 0;
}

static int set_features(struct net_device *dev,
						    netdev_features_t features)
{
	return 0;
}

static void poll_controller(struct net_device *dev)
{
	
}						    	

static const struct net_device_ops dm9000_netdev_ops = {
	.ndo_open		= open,
	.ndo_stop		= stop,
	.ndo_start_xmit		= start_xmit,
	.ndo_tx_timeout		= tx_timeout,
	.ndo_set_rx_mode	= set_rx_mode,
	.ndo_do_ioctl		= do_ioctl,
	.ndo_set_features	= set_features,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address	= eth_mac_addr,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= poll_controller,
#endif
};

/*
 * Search DM9000 board, allocate space and register it
 */
static int
vnet_probe(struct platform_device *pdev)
{
	struct board_info *db;	
	struct net_device *ndev;
	struct device *dev = &pdev->dev;
	const unsigned char *mac_src;
	int ret = 0;
	int iosize;
	int i;
	u32 id_val;
	bool inv_mac_addr = false;

	//分配一个net_driver
	ndev = alloc_etherdev(sizeof(struct board_info));
	if (!ndev)
		return -ENOMEM;

	SET_NETDEV_DEV(ndev, &pdev->dev);

	dev_dbg(&pdev->dev, "myvnet\n");

	/* setup board info structure */
	db = netdev_priv(ndev);

	//不用中断
	ndev->irq = -1;

	/* fill in parameters for net-dev structure */
	ndev->base_addr = -1;

	vnet_reset(db);
	
	ndev->netdev_ops	= &dm9000_netdev_ops;
	ndev->watchdog_timeo	= msecs_to_jiffies(watchdog);
	ndev->ethtool_ops	= &dm9000_ethtool_ops;
	
	platform_set_drvdata(pdev, ndev);
	ret = register_netdev(ndev);

	if (ret == 0) {
		if (inv_mac_addr)
			dev_warn(db->dev, "%s: Invalid ethernet MAC address. Please set using ip\n",
				 ndev->name);
		printk(KERN_INFO "%s: dm9000%c at %p,%p IRQ %d MAC: %pM (%s)\n",
		       ndev->name, dm9000_type_to_char(db->type),
		       db->io_addr, db->io_data, ndev->irq,
		       ndev->dev_addr, mac_src);
	}
	return 0;

out:
	dev_err(db->dev, "not found (%d).\n", ret);

	//vnt_release_board(pdev, db);
	free_netdev(ndev);
	return ret;
}

static int
vnet_drv_suspend(struct device *dev)
{
	
	return 0;
}

static int
vnet_drv_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops vnet_drv_pm_ops = {
	.suspend	= vnet_drv_suspend,
	.resume		= vnet_drv_resume,
};

static int
vnet_drv_remove(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);

	unregister_netdev(ndev);
	free_netdev(ndev);
	dev_dbg(&pdev->dev, "released and freed device\n");
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id vnet_of_matches[] = {
	{ .compatible = "vnet1", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, vnet_of_matches);
#endif

static struct platform_driver vnet_driver = {
	.driver	= {
		.name    = "vnet1",
		.pm	 = &vnet_drv_pm_ops,
		.of_match_table = of_match_ptr(vnet_of_matches),
	},
	.probe   = vnet_probe,
	.remove  = vnet_drv_remove,
};

module_platform_driver(vnet_driver);

MODULE_AUTHOR("Guoke");
MODULE_DESCRIPTION("vnet123");
MODULE_LICENSE("GPL");
