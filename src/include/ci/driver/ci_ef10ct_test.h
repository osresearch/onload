/* SPDX-License-Identifier: GPL-2.0 */
/* X-SPDX-Copyright-Text: (c) Copyright 2023 AMD */

#ifndef CI_DRIVER_CI_EF10CT_TEST_H
#define CI_DRIVER_CI_EF10CT_TEST_H

#include <ci/driver/ci_aux.h>

#define EFX_LLCT_DEVNAME	"llct"
#define EFX_EF10_DEVNAME	"ef10"

/* Driver API */
/** Events a driver can get.
 * @EFX_EVENT_IN_RESET_DOWN: Generated when hardware goes down for reset
 *	The driver must stop all hardware processing before returning from this
 *	method.
 *	Context: process, rntl_lock held
 * @EFX_EVENT_IN_RESET_UP: Generated when hardware is back after reset
 *	If @value is true the driver should resume normal operations. If
 *	@value is false the driver should abandon use of the hardware resources.
 *	@remove will still be called.
 *	Context: process, rntl_lock held
 * @EFX_EVENT_LINK_CHANGE: Physical link changed state
 * @EFX_EVENT_MCDI: MCDI event
 *	An event has been generated by the hardware intended for a queue owned
 *	by this driver. The event is hardware specific and is provided in
 *	@value.
 *	Context: softirq
 */
enum efx_event_type {
	EFX_EVENT_IN_RESET_DOWN,
	EFX_EVENT_IN_RESET_UP,
	EFX_EVENT_LINK_CHANGE,
	EFX_EVENT_MCDI,
};

#define EFX_ALL_EVENTS	(BIT(EFX_EVENT_IN_RESET_DOWN) | \
			 BIT(EFX_EVENT_IN_RESET_UP) | \
			 BIT(EFX_EVENT_LINK_CHANGE) | \
			 BIT(EFX_EVENT_MCDI))

struct efx_auxiliary_event {
	enum efx_event_type	type;
	uint64_t		value;	/**< Link or reset state or event */
};

/** Called when an event on a port may need to be handled by a driver
 * returns NAPI budget consumed.
 */
typedef int (*efx_event_handler)(struct auxiliary_device *dev,
				 void *driver_data,
				 struct efx_auxiliary_event *ev, int budget);

struct efx_auxiliary_device;
struct efx_auxiliary_client;

/* Device API */

/** Remote Procedure Call to the firmware. */
struct efx_auxiliary_rpc {
	unsigned int cmd;
	size_t inlen;
	const u32 *inbuf;
	size_t outlen;
	size_t *outlen_actual;
	u32 *outbuf;
};


/**
 * Device parameters
 *
 * Parameters are returned or set through an appropriate member of the union
 * @efx_auxiliary_param_value.
 *
 * @EFX_AUXILIARY_CONFIG_MEM: PCI memory bar.
 *      Get only.
 *      Returned through @config_mem.
 * @EFX_AUXILIARY_NETDEV: The associated netdev for this interface.
 *      Get only.
 *      Returned through @net_dev.
 * @EFX_AUXILIARY_VARIANT: The HW variant of this interface.
 *      Get only.
 *      Returned through @variant.
 * @EFX_AUXILIARY_REVISION: The HW revision of this interface.
 *      Get only.
 *      Returned through @value.
 * @EFX_AUXILIARY_NIC_RESOURCES: Details of the available queue ranges.
 *      The client must provide the pointer to the structure. On
 *      successful return from get_param() the values will be
 *      populated.
 *      Get only.
 *      Returned through @nic_res.
 * @EFX_AUXILIARY_IRQ_RESOURCES: Details of available irq resources.
 *      The irq resources will correspond to the NIC resources
 *      retrievable via EFX_AUXILIARY_NIC_RESOURCES.
 *      The client must provide the pointer to the structure. On
 *      successful return from get_param() the values will be
 *      populated.
 *      Get only.
 *      Returned through @irq_res.
 * @EFX_AUXILIARY_EVQ_WINDOW: The location of control area for event queues.
 *      The base address is for the event queue evq_min provided through
 *      EFX_AUXILIARY_NIC_RESOURCES. The stride can be used to calculate the
 *      offset of each subsequent event queue from this base.
 *      Get only.
 *      Returned through @evq_window.
 * @EFX_AUXILIARY_CTPIO_WINDOW: The bus address of the CTPIO region for a TXQ
 *      On successful return the provided addr will refer to the IO region, and
 *      size will provide the size of the region.
 *      The returned address should be IO mapped for access to the region.
 *	Get only.
 *	Return through @io_addr
 * @EFX_AUXILIARY_RXQ_POST: The bus address of the RX buffer post register
 *      On successful return the provided addr will refer to the register, and
 *      size will provide the size of the register.
 *      The returned address should be IO mapped for access to the region.
 *	Get only.
 *	Return through @io_addr
 * @EFX_AUXILIARY_DESIGN_PARAMS: Details of supported features that
 *	vary with HW.
 *	Get only.
 *	Returned through @design_params.
 */
enum efx_auxiliary_param {
        EFX_AUXILIARY_CONFIG_MEM,   /**< PCI memory BAR */
	EFX_AUXILIARY_NETDEV,
        EFX_AUXILIARY_VARIANT,
        EFX_AUXILIARY_REVISION,
        EFX_AUXILIARY_NIC_RESOURCES,
        EFX_AUXILIARY_IRQ_RESOURCES,
        EFX_AUXILIARY_EVQ_WINDOW,
        EFX_AUXILIARY_CTPIO_WINDOW,
        EFX_AUXILIARY_RXQ_POST,
        EFX_AUXILIARY_DESIGN_PARAM,
};


struct efx_auxiliary_design_params {
        /* stride between entries in receive window */
        u32 rx_stride;
        /* Length of each receive buffer */
        u32 rx_buffer_len;
        /* Maximum Rx queues available */
        u32 rx_queues;
        /* Maximum Tx apertures available */
        u32 tx_apertures;
        /* Maximum number of receive buffers can be posted */
        u32 rx_buf_fifo_size;
        /* Fixed offset to the frame */
        u32 frame_offset_fixed;
        /* Receive metadata length */
        u32 rx_metadata_len;
        /* Largest window of reordered writes to the CTPIO */
        u32 tx_max_reorder;
        /* CTPIO aperture length */
        u32 tx_aperture_size;
        /* Size of packet FIFO per CTPIO aperture */
        u32 tx_fifo_size;
        /* partial time stamp in sub nano seconds */
        u32 ts_subnano_bit;
        /* Width of sequence number in EVQ_UNSOL_CREDIT_GRANT register */
        u32 unsol_credit_seq_mask;
        /* L4 csum fields */
        u32 l4_csum_proto;
        /* Max length of frame data when LEN_ERR indicates runt*/
        u32 max_runt;
        /* Event queue sizes */
        u32 evq_sizes;
        /* Number of filters */
        u32 num_filter;
};


/**
 * Interrupt resource information
 *
 * @flags: currently none
 * @n_ranges: Number of entries in irq_ranges. Must be > 0.
 * @int_prime: Address of the INT_PRIME register.
 * @irq_ranges: Array of interrupts, specified as base vector + range.
 */
struct efx_auxiliary_irq_resources {
        u16 flags;
        u16 n_ranges;
        void __iomem *int_prime;
        struct efx_auxiliary_irq_range {
                int vector;
                int range;
        } irq_ranges[1];
};

/**
 * Queue resource information
 *
 * @evq_min: index of first available event queue
 * @evq_lim: index of last available event queue + 1
 */
struct efx_auxiliary_nic_resources {
        unsigned int evq_min;
        unsigned int evq_lim;
        unsigned int txq_min;
        unsigned int txq_lim;
        unsigned int rxq_min;
        unsigned int rxq_lim;
};


/**
 * Location of event queue control window.
 *
 * @base: physical address of base of the event queue window
 * @stride: size of each event queue's region within the window
 */
struct efx_auxiliary_evq_window {
        resource_size_t base;
        size_t stride;
};


/** Location of an IO area associated with a queue.
 * @base: bus address of base of the region
 * @size: size of this queue's region
 */
struct efx_auxiliary_io_addr {
	int qid_in;
        resource_size_t base;
        size_t size;
};


/** Possible values for device parameters */
union efx_auxiliary_param_value {
        void __iomem *config_mem;
	struct net_device *net_dev;
        struct efx_auxiliary_irq_resources *irq_res;
        struct efx_auxiliary_nic_resources nic_res;
        struct efx_auxiliary_evq_window evq_window;
        struct efx_auxiliary_design_params design_params;
	struct efx_auxiliary_io_addr io_addr;
        char variant;
        int value;
        void *ptr;
        bool b;
};


#define EFX_AUXILIARY_QUEUE_ALLOC	-1
#define EFX_AUXILIARY_QUEUE_DONT_ALLOC	-2
/**
 * The parameters necessary to request allocation of a set of LL queues.
 *
 * @n_queue_sets: The number of entries in the q_sets array
 * @q_sets: Each set comprises a set of queue resources, of which any
 *          combination can be requested to be allocated.
 */
struct efx_auxiliary_queues_alloc_params {
        int n_queue_sets;
        struct efx_auxiliary_queue_set {
                int evq;
                int txq;
                int rxq;
		int irq;
        } q_sets[1];
};


/**
 * Device operations.
 *
 * @open: Clients need to open a device before using it. This allocates a
 *	  client ID used for further operations, and can register a callback
 *	  function for events. events_requested is a bitmap of
 *	  enum efx_event_type.
 * @close: Closing a device stops it from getting events and frees client
 *	   resources.
 * @fw_rpc: Remote procedure call to the firmware.
 * @get_param: Obtains the efx_auxiliary_param.
 * @set_param: Sets the efx_auxiliary_param.
 *      See the documentation for each paramter type in @efx_auxiliary_param for
 *      details of each available parameter.
 * @queues_alloc: Allocate a set of queues for use by this client.
 *	Once allocated the client is responsible for initialisation and tear
 *	down of allocated queues as needed.
 *      Returns 0 on success, error otherwise.
 */
struct efx_auxiliary_devops {
	struct efx_auxiliary_client *(*open)(struct auxiliary_device *auxdev,
					     efx_event_handler func,
					     unsigned int events_requested,
					     void *driver_data);
	int (*close)(struct efx_auxiliary_client *handle);
	int (*fw_rpc)(struct efx_auxiliary_client *handle,
		      struct efx_auxiliary_rpc *rpc);
        int (*get_param)(struct efx_auxiliary_client *handle,
			 enum efx_auxiliary_param p,
                         union efx_auxiliary_param_value *arg);
        int (*set_param)(struct efx_auxiliary_client *handle,
			 enum efx_auxiliary_param p,
                         union efx_auxiliary_param_value *arg);
	int (*queues_alloc)(struct efx_auxiliary_client *handle,
			    struct efx_auxiliary_queues_alloc_params *params);
	int (*queues_free)(struct efx_auxiliary_client *handle,
			   struct efx_auxiliary_queues_alloc_params *params);
#if 0
	int (*filter_insert)(struct efx_auxiliary_client *handle,
			     struct efx_auxiliary_filter_params *params);
	int (*filter_remove)(struct efx_auxiliary_client *handle,
			     unsigned int filter_id);
	int (*filter_redirect)(struct efx_auxiliary_client *handle,
			     struct efx_auxiliary_filter_params *params);
        int (*filter_block_kernel)(struct efx_auxiliary_client *handle,
                                   enum efx_dl_filter_block_kernel_type block);
        void (*filter_unblock_kernel)(struct efx_auxiliary_client *handle,
                                      enum efx_dl_filter_block_kernel_type type);
        int (*set_multicast_loopback_suppression)(struct efx_auxiliary_client *handle,
        u32 (*rss_flags_default)(struct efx_auxiliary_client *handle);
        int (*rss_context_new)(struct efx_auxiliary_client *handle,
                               const u32 *indir, const u8 *key,
                               u32 flags, u8 num_queues,
                               u32 *rss_context);
        int (*rss_context_set)(struct efx_auxiliary_client *handle,
                               const u32 *indir, const u8 *key,
                               u32 flags, u32 rss_context);
        int (*rss_context_free)(struct efx_auxiliary_client *handle,
                               u32 rss_context);
        int (*vport_new)(struct efx_auxiliary_client *handle, u16 vlan,
                         bool vlan_restrict);
        int (*vport_free)(struct efx_auxiliary_client *handle, u16 port_id);
                                                  bool suppress, u16 vport_id,
                                                  u8 stack_id);
#endif


};

/**
 * Auxiliary device interface.
 *
 * @vdev: The parent auxiliary bus device.
 * @ops: Device API.
 */
struct efx_auxiliary_device {
	struct auxiliary_device auxdev;
	const struct efx_auxiliary_devops *ops;
};

static inline
struct efx_auxiliary_device *to_sfc_aux_device(struct auxiliary_device *adev)
{
	return container_of(adev, struct efx_auxiliary_device, auxdev);
}


/* FIXME SCJ these structs are not really part of the interface, they're just
 * here to make it easier to transition the queue init to MCDI. */
struct efx_auxiliary_evq_params {
        int qid;
        int irq;
        int entries;
        struct page *q_page;
        size_t page_offset;
        size_t q_size;
        u32 flags;
        bool subscribe_time_sync;
        u16 unsol_credit;
};


/**
 * The parameters necessary to request a TX queue.
 *
 * @evq: The event queue to associate with the allocated TXQ.
 */
struct efx_auxiliary_txq_params {
        int evq;
        int label;
        int qid;
};

/**
 * The parameters necessary to request an RX queue.
 *
 * @evq: The event queue to associate with the allocated RXQ.
 */
struct efx_auxiliary_rxq_params {
        int evq;
        int label;
        int qid;
};

#endif /* CI_DRIVER_CI_EF10CT_TEST_H */
