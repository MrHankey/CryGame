#ifndef _JOBMANAGER_SPUDRIVER_H_
#define _JOBMANAGER_SPUDRIVER_H_

#if !defined(_ALIGN)
// needed for SPU compilation (should be removed as soon as the spus doesn't depend on this header anymore
#if defined(_MSC_VER)
	#define _ALIGN(x)
#else
	#define _ALIGN(x) __attribute__((aligned(x)))
#endif
#endif

// ==============================================================================
// Job manager settings
// ==============================================================================
//enable to obtain stats of spu usage each frame
#define SUPP_SPU_FRAME_STATS

// collect per job informations about dispatch, start, stop and sync times
#if ! defined(DEDICATED_SERVER)
#define JOBMANAGER_SUPPORT_PROFILING
#endif

// store the latest fnresolv, stackframe and dma access
#define JOBMANAGER_STORE_DEBUG_HELP_INFORMATIONS

// support capturing SPU-Threads in tuner.
#define JOBMANAGER_SUPPORT_TUNER_CAPTURES

// in release disable features which cost performance
#if defined(_RELEASE)
#undef SUPP_SPU_FRAME_STATS
#undef JOBMANAGER_SUPPORT_PROFILING
#undef JOBMANAGER_STORE_DEBUG_HELP_INFORMATIONS
#undef JOBMANAGER_SUPPORT_TUNER_CAPTURES
#endif


// ==============================================================================
// Common Job manager defines
// ==============================================================================
//ppu event calls
//highest bit set for event cond
#define EVENT_COND (1 << 31)
#define EVENT_OPCODE_MASK (7)
#define EVENT_ADRESS_SHIFT 1
#define EVENT_ADRESS_MASK ~(EVENT_COND | EVENT_OPCODE_MASK)
#define eEOC_CondNotify 0
//for now NotifySingle is just an alias for Notify - this _might_ change some day.
#define eEOC_CondNotifySingle 0
#define eEOC_CondDestroy 1
#define eEOC_ReleaseSemaphore 2
#define eEOC_Unused 3
#define eEOC_JobStateSetStopped 4
// when adding new values here, make sure the OPCODE_MASK is big enough
// don't add to many, these are stored in the lower bits of a 16 byte aligned value

// ==============================================================================
// Common Job manager enums (needed to be ifdefs since this file is included by asm files
// ==============================================================================
// don't include enums and structures when compiling asm files
#if !defined(SPU_DRIVER_ASM_FILE)
namespace JobManager {
	enum { SPU_EVENT_QUEUE_CHANNEL = 42 }; // channel id used for SPU -> PPU communication
	enum { SPU_EVENT_QUEUE_NUM_ENTRIES = 32 } ; // number of slots in the event queue from SPU to PPU

namespace SPUBackend {
	
	enum { USER_DMA_TAG_BASE = 12 };				// base tag available to custom usage
	enum { MEM_TRANSFER_DMA_TAG_BASE	= USER_DMA_TAG_BASE+2 };	//tag available for memtransfer_from_main/memtransfer_to_main

	//page mode
	enum EPageMode
	{
		ePM_Single = 0,		//single page mode, job occupies as much as it takes
		ePM_Dual	 = 1,		//2 pages
		ePM_Quad	 = 2,		//4 pages
	};
}
}


namespace JobManager {
	// ------ profling structs ------ //
	//single statistic for an SPU, 16 byte and 16 byte aligned for DMA
	struct SSingleSPUStat
	{
		unsigned int lock;									//rw lock (write only PPU, read SPU)
		unsigned int count[6];							//running stats for SPUs, one SPU busy decrementer count for each SPU
		unsigned int dummy;									//was: curSPUPivot;		current pivot ID (to have the same SPU ID workflow per frame)
		unsigned int lockPad[128-8-(6<<2)];	//keep cacheline clean


		inline SSingleSPUStat();


	} _ALIGN(128);//DMA relevant

	SSingleSPUStat::SSingleSPUStat() : lock(0)
	{};


	///////////////////////////////////////////////////////////////////////////////
	// util structure for acks/data send back to SPU
	// since event queue try_recieve is very expensive on SPU
	// we are using a atomic cacheline read
	struct SSPUEventAcknowledgeLine
	{
		enum { nAcknowledgeCleared = 0};
		enum { nAcknowledgeSend = 1};

		unsigned int nAck;					// ack value, set to != 0 to wake up SPU
		unsigned int nReturnValue;	// value to return to the SPU
	} _ALIGN(128);

} // namespace JobManager

#endif

#endif // _JOBMANAGER_SPUDRIVER_H_