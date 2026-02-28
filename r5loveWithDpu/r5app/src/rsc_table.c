/* rsc_table.c - ����汾�������� DDR */
#include <openamp/open_amp.h>
#include "rsc_table.h"

/* Place resource table in special ELF section */
#define __section_t(S)          __attribute__((__section__(#S)))
#define __resource              __section_t(.resource_table)

#define RPMSG_IPU_C0_FEATURES        1
#define VIRTIO_ID_RPMSG_             7
#define VIRTIO_RPMSG_F_NS           1  /* 启用 namespace announcement */
#define NUM_VRINGS                  0x02
#define VRING_ALIGN                 0x1000
//#define RING_TX                     FW_RSC_U32_ADDR_ANY
//#define RING_RX                     FW_RSC_U32_ADDR_ANY
#define VRING_SIZE                  256
#define RING_TX 0x3fd00000
#define RING_RX 0x3fd04000
#define VRING_ALIGN 0x1000
#define VRING_SIZE  256


/* ֻ��һ����Դ��VDEV */
#define NUM_TABLE_ENTRIES           1

struct remote_resource_table {
	unsigned int version;
	unsigned int num;
	unsigned int reserved[2];
	unsigned int offset[NUM_TABLE_ENTRIES];
    /* ɾ�� ddr_mem��ֻ�� VDEV */
	struct fw_rsc_vdev rpmsg_vdev;
	struct fw_rsc_vdev_vring rpmsg_vring0;
	struct fw_rsc_vdev_vring rpmsg_vring1;
} __attribute__((packed, aligned(0x100)));

struct remote_resource_table __resource resources = {
	/* Version */
	1,
	/* Number of table entries */
	NUM_TABLE_ENTRIES,
	/* reserved fields */
	{0, 0},
	/* Offsets */
	{
		offsetof(struct remote_resource_table, rpmsg_vdev),
	},
	/* Resource: Virtio Device */
	{
		RSC_VDEV, VIRTIO_ID_RPMSG_, 0, RPMSG_IPU_C0_FEATURES, 0, 0, 0,
		NUM_VRINGS, {0, 0},
	},
	/* Vrings */
	{RING_TX, VRING_ALIGN, VRING_SIZE, 1, 0},
	{RING_RX, VRING_ALIGN, VRING_SIZE, 2, 0},
};

void *get_resource_table (int rsc_id, int *len)
{
	(void) rsc_id;
	*len = sizeof(resources);
	return &resources;
}
