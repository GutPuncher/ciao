/*
 *  eng.h
 *
 *  Common definitions for writing Ciao engine code
 *
 *  See Copyright Notice in ciaoengine.pl
 */

#ifndef _CIAO_ENG_H
#define _CIAO_ENG_H

#include <ciao/eng_predef.h>
#include <ciao/os_threads.h>

/* --------------------------------------------------------------------------- */
/* (default options) */

/* (for some reason this makes the engine around 6% faster) */
//#define USE_TAGGED_CHOICE_START

// uncomment to enable C version of the multi-attributes accessors.
// #define USE_FAST_MULTIATTR 1

/* =========================================================================== */
/* Margins and initial allocation parameters. */

#define kCells   1024

#define STATICMAXATOM 1024     /* Avoid very long atoms inside the engine */

#define MAXATOM Atom_Buffer_Length /* TODO: used? */

#define CONTPAD 128*sizeof(tagged_t) /* min. amount of heap at proceed */
// TODO: MAXATOM is dynamic??!! why MAXATOM here??
//#define CALLPAD (2*(MAXATOM)*sizeof(tagged_t) + CONTPAD)
#define CALLPAD (2*(STATICMAXATOM)*sizeof(tagged_t) + CONTPAD)  /* min. amount of heap at call */

/* TODO: When does CALLPAD really need dynamic MAXATOM? Avoid it if possible */
/* Static version of CALLPAD (should be the same value used in plwam) */
#define STATIC_CALLPAD (2*(STATICMAXATOM)*sizeof(tagged_t) + CONTPAD)

/* min. amount of stack at allocate */
#define STACKPAD (2*ARITYLIMIT + 16)

/* min. amount of trail/choice at try */
#define CHOICEPAD (2*ARITYLIMIT)

#define ATMTABSIZE  (4*kCells)  /* size of global atom table  */
#define QLOADSIZE   (2*kCells)  /* plenty at present */

#if (defined(ANDPARALLEL) || defined(PARBACK))
#define GLOBALSTKSIZE   (600*kCells-1) /* Was 6*kCells-1 (DCG) */
#define LOCALSTKSIZE    (300*kCells-1)
#define CHOICESTKSIZE   (300*kCells-1)
#define TRAILSTKSIZE    (300*kCells-1)
#else
#define GLOBALSTKSIZE   (16*kCells-1) /* Was 6*kCells-1 (DCG) */
#define LOCALSTKSIZE    (4*kCells-1)
#define CHOICESTKSIZE   (4*kCells-1)
#define TRAILSTKSIZE    (4*kCells-1)
#endif

#define XREGBANKSIZE    ARITYLIMIT

/* =========================================================================== */
/* Principal WAM areas and registers. */

/* Access macros for the principal WAM registers, bytecode offsets, etc. */

/*
  Macros CIAO_REG_n, used to declare a variable and its type, force
  the GCC compiler to store a variable in a register.  In principle,
  they are ranked in ascending order of effectiveness, according to
  empiric measurements.  This can of course change if compilers (GCC,
  in this case) change the way registers are assigned, and it makes
  sense to have an order because there are processors whose
  instruction set is not orthogonal to register names.  n currently
  goes from 1 to 4, but it can be enlarged for additional
  architectures.

  Undefined the macro MAP_TO_HW_REGISTERS to disable this mapping a
  fall back to what the compiler thinks is reasonable.

  Register assignments (for registers 1 to 3) have been made for i686.
*/

#if defined(__clang__)
/* clang/llvm does not yet support explicit registers variables */
#else
/* Temporarily DISABLED:

   It seems to be broken in GCC 6 (Ubuntu 17). A fix consinst on
   disabling the use of 'ebx' register:
   
     // #define CIAO_REG_2(Type, Var) register Type Var asm("ebx")
     #define CIAO_REG_2(Type, Var) Type Var

   However, performance (using ecrc benchmark) does not seem to be
   better on the general case (although it makes some particular
   benchmarks faster). E.g., geometrical average KLIPS

     32bit with HW regs    -> 17533.91836
     32bit without HW regs -> 19492.57422
     64bit without HW regs -> 23928.04371

   We will keep this optimization temporarily disabled until we find a
   better assignment.

   MCL & JF
*/
/* #define MAP_TO_HW_REGISTERS */
#endif

/* TODO: missing x86_64 */
#if defined(__GNUC__) && defined(MAP_TO_HW_REGISTERS) 
# if defined(i686) && (defined(LINUX) || defined(BSD))
#  define CIAO_REG_1(Type, Var) register Type Var asm("esi")
#  define CIAO_REG_2(Type, Var) register Type Var asm("ebx")
#  define CIAO_REG_3(Type, Var) register Type Var asm("edi")
#  define CIAO_REG_4(Type, Var) Type Var
# elif defined(i686) && defined(DARWIN)
#  define CIAO_REG_1(Type, Var) register Type Var asm("esi")
#  define CIAO_REG_2(Type, Var) register Type Var asm("edi")
#  define CIAO_REG_3(Type, Var) Type Var
#  define CIAO_REG_4(Type, Var) Type Var
# else
#  define CIAO_REG_1(Type, Var) Type Var
#  define CIAO_REG_2(Type, Var) Type Var
#  define CIAO_REG_3(Type, Var) Type Var
#  define CIAO_REG_4(Type, Var) Type Var
# endif
#else
# define CIAO_REG_1(Type, Var) Type Var
# define CIAO_REG_2(Type, Var) Type Var
# define CIAO_REG_3(Type, Var) Type Var
# define CIAO_REG_4(Type, Var) Type Var
#endif

#define Arg w
#if defined(OPTIM_COMP)
#define G (&w->g)
#else
#define G w
#endif

#define WToX0           (SIZEOF_FLEXIBLE_STRUCT(worker_t, tagged_t, 0)/sizeof(tagged_t))
#define Xb(I)           (*CharOffset(w,I)) /* I as bytecode operand */
#define X(I)            (w->x[I]) /* I as zero-based */
#define Xop(X)  (((X)+WToX0)*sizeof(tagged_t))
#define Xinv(X) (((X)/sizeof(tagged_t))-WToX0)

/* # X regs used for control in choicepoints for dynamic code */
/*
   X(0) -
   X(1) -
   X(2) - x2_next clause pointer / handle.
   X(3) -
   X(4) - clock (used even in conc. predicates, although ignored).
   X(5) - x5_next clause pointer / handle.
   ------ The next ones, only meaningful for concurrent predicates.
   X(6) - predicate root (needed in case there are no clause pointers - MCL).
   X(7) - blocking/non-blocking and exited/non exited (MCL).
   X(8) - pointer to previous dynamic concurrent choicepoint.
*/

#define X2_CHN 2
#define ClockSlot 4
#define X5_CHN 5
#define RootArg 6
#define InvocationAttr 7
#define PrevDynChpt 8
#define DynamicPreserved 9

/* NOTE: was 1<<0 and 1<<1; avoid reserved bits */
#define BLOCKIDX ((intmach_t)1<<tagged__num_offset)
#define EXECIDX  ((intmach_t)1<<(tagged__num_offset+1))

#define SET_BLOCKING(arg) (arg) = ((arg) | BLOCKIDX)
#define SET_NONBLOCKING(arg) (arg) = ((arg) & ~BLOCKIDX)
#define IS_BLOCKING(arg) ((arg) & BLOCKIDX)
#define IS_NONBLOCKING(arg) !((arg) & BLOCKIDX)

#define SET_EXECUTING(arg) (arg) = ((arg) | EXECIDX)
#define SET_NONEXECUTING(arg) (arg) = ((arg) & ~EXECIDX)
#define EXECUTING(arg) ((arg) & EXECIDX)
#define NONEXECUTING(arg) !((arg) & EXECIDX)


/* initial choicepoint */
#define InitialChoice ChoiceNext0(Choice_Start,1)

#define EToY0           (SIZEOF_FLEXIBLE_STRUCT(frame_t, tagged_t, 0)/sizeof(tagged_t))
#define Yb(I)           (*CharOffset(E,I)) /* I as bytecode operand */
#define Y(I)            (E->x[I]) /* I as zero-based */
#define Yop(X)  (((X)+EToY0)*sizeof(tagged_t))
#define Yinv(X) (((X)/sizeof(tagged_t))-EToY0)

#define FrameSize(L) BCOp0((L), FTYPE_ctype(f_e), -FTYPE_size(f_e))
#define FrameSizeToCount(O)     ((O)/sizeof(tagged_t)-EToY0)

/* Private areas for a thread, related to the overall wam status */

#if defined(TABLING)
#define StackFReg                       (w->misc->stack_freg)
#define HeapFReg                        (w->misc->heap_freg)
#define FirstNodeTR(Node)               ((node_tr_t*)(Node)->local_top)
#define SetFirstNodeTR(Node,Value)      ((Node)->local_top = (frame_t*)(Value))
#define LastNodeTR                      (w->misc->last_node_tr)
#define FrozenChpt(Node)   ((Node)->heap_top == (tagged_t*)&(HeapFReg))
#define NODE_TR_SIZE(NodeTR)            (((node_tr_t*)(NodeTR))->size)
#define NODE_TR_TRAIL_SG(NodeTR)        (((node_tr_t*)(NodeTR))->trail_sg)
#define NODE_TR_NEXT(NodeTR)            (((node_tr_t*)(NodeTR))->next)
#define NODE_TR_CHAIN(NodeTR)           (((node_tr_t*)(NodeTR))->chain)
#endif

#if defined(ANDPARALLEL)
#define Goal_List_Start                   (w->misc->goal_list_start)
#define Goal_List_Top                     (w->misc->goal_list_top)
#define Goal_Cache                        (w->misc->goal_cache)
#define Dep_Size                          (w->misc->dep_size_exec)
#define Dep_Id                            (w->misc->dep_id_exec)
#define Goal_List_Lock                    (w->misc->goal_list_l)
#define Event_Queue_Start                 (w->misc->event_queue_start)
#define Event_Queue_Top                   (w->misc->event_queue_top)
#define Event_Queue_Lock                  (w->misc->event_queue_l)
#define Last_Parallel_Exec                (w->misc->last_parallel_exec)
#define Cancel_Goal_Exec                  (w->misc->cancel_goal_exec)
#define Safe_To_Cancel                    (w->misc->safe_to_cancel)
#define Mutex_Lock                        (w->misc->mutex_l)
#define Mode                              (w->misc->mode)
#define Waiting_For_Work_Cond_Var         (w->misc->waiting_for_work_cv)
#define Waiting_For_Work_Lock             (w->misc->waiting_for_work_l)
#define Suspended_Waiting_For_Work        (w->misc->suspended_waiting_for_work)
#define Suspend                           (w->misc->suspend)
#if defined(Solaris)
#define Total_Suspending_Time             (w->misc->total_suspending_time)
#define Suspending_Time_Cont              (w->misc->suspending_time_cont)
#endif
#define Next_Wam                          (w->misc->next_wam)

#define Goal_List_Start_Of(x)             (x->misc->goal_list_start)
#define Goal_List_Top_Of(x)               (x->misc->goal_list_top)
#define Goal_Cache_Of(x)                  (x->misc->goal_cache)
#define Dep_Size_Of(x)                    (x->misc->dep_size_exec)
#define Dep_Id_Of(x)                      (x->misc->dep_id_exec)
#define Goal_List_Lock_Of(x)              (x->misc->goal_list_l)
#define Event_Queue_Start_Of(x)           (x->misc->event_queue_start)
#define Event_Queue_Top_Of(x)             (x->misc->event_queue_top)
#define Event_Queue_Lock_Of(x)            (x->misc->event_queue_l)
#define Last_Parallel_Exec_Of(x)          (x->misc->last_parallel_exec)
#define Cancel_Goal_Exec_Of(x)            (x->misc->cancel_goal_exec)
#define Safe_To_Cancel_Of(x)              (x->misc->safe_to_cancel)
#define Mutex_Lock_Of(x)                  (x->misc->mutex_l)
#define Mode_Of(x)                        (x->misc->mode)
#define Waiting_For_Work_Cond_Var_Of(x)   (x->misc->waiting_for_work_cv)
#define Waiting_For_Work_Lock_Of(x)       (x->misc->waiting_for_work_l)
#define Suspended_Waiting_For_Work_Of(x)  (x->misc->suspended_waiting_for_work)
#define Suspend_Of(x)                     (x->misc->suspend)
#if defined(Solaris)
#define Total_Suspending_Time_Of(x)       (x->misc->total_suspending_time)
#define Suspending_Time_Cont_Of(x)        (x->misc->suspending_time_cont)
#endif
#define Next_Wam_Of(x)                    (x->misc->next_wam)

/* Value types for controlling agents execution when expanding stack
   sets */
#define RELEASED                          0
#define TOSUSPEND                         1
#define SUSPENDED                         2
#define WAITING                           3

/* Accion types for releasing execution of deterministic parallel goals */
#define FORWARD_EXEC                      0
#define UNWIND                            1

#if defined(VISANDOR)
#define Count_Calls(x)                    (x->misc->my_count_calls)
#define Time_Each_Call(x)                 (x->misc->my_time_each_call)
#define Pcall_Level(x)                    (x->misc->pcall_level)
#define TmpEvPtr(x)                       (x->misc->tmpevptr)
#define NextEvent(x)                      (x->misc->nextevent)
#define FirstEvent(x)                     (x->misc->firstevent)
#define LastEvent(x)                      (x->misc->lastevent)

/* Event types for tracing tool VisAndOr */
#define FORK         1
#define START_GOAL   2
#define FINISH_GOAL  3
#define JOIN         4

#define START_TIME   5
#define STOP_TIME    6

#define AGENT_BUSY   7  /* working */
#define AGENT_IDLE   8  /* waiting for work */

// Never used by VisAndOr!
// #define CREATE_WAM   9
// #define CREATE_AGENT 10
#endif

#endif

#if defined(PARBACK)
#define Act_PF                            (w->misc->pf)
#define Goal_List_Start                   (w->misc->goal_list_start)
#define Goal_List_Top                     (w->misc->goal_list_top)
#define Cancel_Goal_List_Start            (w->misc->cancel_goal_list_start)
#define Cancel_Goal_List_Top              (w->misc->cancel_goal_list_top)
#define Back_Goal_List_Start              (w->misc->back_goal_list_start)
#define Back_Goal_List_Top                (w->misc->back_goal_list_top)
#define Goal_To_Cancel                    (w->misc->goal_to_cancel)
#define Safe_To_Cancel                    (w->misc->safe_to_cancel)
#define Mutex_Lock                        (w->misc->mutex_l)
#define Waiting_For_Work_Cond_Var         (w->misc->waiting_for_work_cv)
#define Waiting_For_Work_Lock             (w->misc->waiting_for_work_l)
#define Suspended_Waiting_For_Work        (w->misc->suspended_waiting_for_work)
#define Suspend                           (w->misc->suspend)
#define Next_Wam                          (w->misc->next_wam)

#define Goal_List_Start_Of(x)             (x->misc->goal_list_start)
#define Goal_List_Top_Of(x)               (x->misc->goal_list_top)
#define Cancel_Goal_Exec_Handler_Of(x)    (x->misc->cancel_goal_exec)
#define Safe_To_Cancel_Of(x)              (x->misc->safe_to_cancel)
#define Mutex_Lock_Of(x)                  (x->misc->mutex_l)
#define Waiting_For_Work_Cond_Var_Of(x)   (x->misc->waiting_for_work_cv)
#define Waiting_For_Work_Lock_Of(x)       (x->misc->waiting_for_work_l)
#define Suspended_Waiting_For_Work_Of(x)  (x->misc->suspended_waiting_for_work)
#define Suspend_Of(x)                     (x->misc->suspend)
#define Next_Wam_Of(x)                    (x->misc->next_wam)

/* Value types for controlling agents execution when expanding stack
   sets */
#define RELEASED                          0
#define TOSUSPEND                         1
#define SUSPENDED                         2
#define WAITING                           3

#endif

#define Heap_Start          w->heap_start
#define Heap_End            w->heap_end
#define Stack_Start         w->stack_start
#define Stack_End           w->stack_end
#define Stack_Warn          ((tagged_t *)StackOffset(Stack_End,-STACKPAD))
//#define Stack_Warn          w->stack_warn
#define Choice_End          w->choice_end
#define Choice_Start        w->choice_start

#if defined(USE_TAGGED_CHOICE_START)
#define Tagged_Choice_Start w->tagged_choice_start
#endif

#define Trail_Start         w->trail_start
#define Trail_End           w->trail_end


/* These are related to the I/O pointers */

#define Input_Stream_Ptr    w->streams->input_stream_ptr
#define Output_Stream_Ptr   w->streams->output_stream_ptr
#define Error_Stream_Ptr    w->streams->error_stream_ptr

/* These keep the current state of the debugger. */

#define Current_Debugger_State w->debugger_info->current_debugger_state
#define Current_Debugger_Mode  w->debugger_info->current_debugger_mode


/* Local space to generate atoms and other general string operations */

#define Atom_Buffer          w->atom_buffer
#define Atom_Buffer_Length   w->atom_buffer_length


/* Incore compiling */

#define Last_Insn            (w->last_insn)

/* Expanded worker (not always active) */

#define Expanded_Worker (w->misc->expanded_worker)

#define Next_Worker(w)  (w->misc->next_worker)

#define Stop_This_Goal(w) (w->misc->stop_this_goal)

// TODO: better place to store this bit?
#define IsSuspendedGoal(w) ((bool_t)((intptr_t)(w->dummy0)))
#define SetSuspendedGoal(w,S) (w->dummy0 = (void *)(S))

/* Global variables */

#define GLOBVAR(i) w->misc->globalvar[i]

/* The local (per-thread) definitions for garbage collection */

#define Gc_Total_Grey    (w->misc->gc_total_grey)
#define Gcgrey           (w->misc->gcgrey)
#define Total_Found      (w->misc->total_found)
#define Cvas_Found       (w->misc->cvas_found)
#define Gc_Aux_Choice      (w->misc->gc_aux_node)
#define Gc_Choice_Start  (w->misc->gc_choice_start)
#define Gc_Heap_Start    (w->misc->gc_heap_start)
#define Gc_Stack_Start   (w->misc->gc_stack_start)


/* Topmost choicepoint for calls to concurrent facts. */

#define TopConcChpt     w->misc->top_conc_chpt

/* Global registers */

#define GlobReg(Which) w->misc->globalreg[Which]

/* Throwing exceptions */

#define ErrArgNo w->misc->errargno
#define ErrCode w->misc->errcode
#define ErrFuncName w->misc->errfuncname
#define ErrFuncArity w->misc->errfuncarity
#define Culprit w->misc->culprit

/* TODO:[ec-merge] try another way and measure performance impact */
#if defined(TABLING)
#define NodeLocalTop(Node)                                      \
  (((Node)->heap_top != (tagged_t*)(&(HeapFReg))) ?           \
   (Node)->local_top : StackFReg)
#define NodeGlobalTop(Node)                                     \
  (((Node)->heap_top != (tagged_t*)(&(HeapFReg))) ?           \
   (Node)->heap_top : HeapFReg)
#else
#define NodeLocalTop(Node)  (Node)->local_top
#define NodeGlobalTop(Node) (Node)->heap_top
#endif

#define GetFrameTop(A,B,Frame) do { \
  if (w->local_top) { \
    (A) = (typeof(A))w->local_top; \
  } else { \
    (A) = (typeof(A))NodeLocalTop(B); \
    if (!StackYounger((A),(Frame))) { \
      (A) = (typeof(A))StackCharOffset((Frame),FrameSize(w->next_insn)); \
    } \
  } \
} while(0);
#define CODE_ALLOC(Frame) GetFrameTop((Frame),w->choice,G->frame)

#define UpdateLocalTop(B,Frame) GetFrameTop(w->local_top,(B),(Frame))

#define NewShadowregs(Gtop) \
{ \
  w->global_uncond = Tagp(HVA,Gtop); \
  w->local_uncond = Tagp(SVA,w->local_top); \
}

// TODO:[oc-merge] G->next_alt must be up to date when failcont uses it (not merged yet)
#if defined(USE_DEEP_FLAGS)
#define SetChoice(Chpt) do { \
  G->next_alt = (Chpt)->next_alt; \
  SetChoice0((Chpt)); \
} while(0)
#else
#define SetChoice(Chpt) do { \
  SetChoice0((Chpt)); \
} while(0)
#endif
#define SetChoice0(Chpt) do { \
  w->choice = (Chpt); \
  w->global_uncond = Tagp(HVA,NodeGlobalTop(w->choice)); \
  w->local_uncond = Tagp(SVA,NodeLocalTop(w->choice)); \
} while(0)

#if defined(ANDPARALLEL) && defined(VISANDOR)
/* Event output macros for tracing tool VisAndOr */

#if defined(SYMM)
#define USCLK_EXISTS
#undef  NSCLK_EXISTS
#define TIME_INIT  usclk_init()
#define TIME       ((float)getusclk() - time_at_event_start*1e6)
#endif

/* #if defined(Solaris) */
#if defined(DARWIN)||defined(LINUX)||defined(EMSCRIPTEN)||defined(Solaris)||defined(Win32)||defined(BSD)
#undef  USCLK_EXISTS
#define NSCLK_EXISTS
#define TIME_INIT   
#define TIME   ((gethrtime()/1000) - (time_at_event_start*1e6))
#endif

#define EVENT(Event, PPF, NFork) \
 { \
   if ( gen_event_file ) { \
     if ( (TmpEvPtr(w) = (NextEvent(w))++) == LastEvent(w) ) { \
          fprintf(stderr, "Agent 0x%p event table overflow.\n", w); \
          gen_event_file = FALSE; \
     } \
     else { \
          TmpEvPtr(w)->timestamp = (uintmach_t)TIME; \
          TmpEvPtr(w)->evtype = (Event); \
          TmpEvPtr(w)->ppf = (PPF); \
          TmpEvPtr(w)->nfork = (NFork); \
          TmpEvPtr(w)->wam = w; \
     } \
   } \
 }
#endif

#if defined(TABLING)
#define ON_TABLING(X) X
#else
#define ON_TABLING(X)
#endif

#if defined(ANDPARALLEL)
#define ON_ANDPARALLEL(X) X
#else
#define ON_ANDPARALLEL(X)
#endif

#if defined(DEBUG)
#define ON_DEBUG(X) X
#else
#define ON_DEBUG(X)
#endif

#if defined(DEBUG_NODE)
#define ON_DEBUG_NODE(X) X
#else
#define ON_DEBUG_NODE(X)
#endif

#define TEST_CHOICE_OVERFLOW(B, AMOUNT) ({ \
  if (ChoiceYounger((B),TrailOffset(w->trail_top,(AMOUNT)))) { \
    CVOID__CALL(choice_overflow, 2*(AMOUNT)*sizeof(tagged_t), TRUE); \
  } \
})

tagged_t deffunctor(char *pname, int arity); /* eng_registry.c */

// TODO: missing test_choice_overflow
// TODO: OPTIM_COMP saves local_top here, not in NECK_TRY
#define CODE_CHOICE_NEW(B, ALT) ({ \
  GetFrameTop(w->local_top,w->choice,G->frame); /* get_frame_top */ \
  CODE_CHOICE_NEW00(B, w->choice, ALT, G->heap_top); \
})
#define CODE_CHOICE_NEW0(B, ALT, H) CODE_CHOICE_NEW00(B, B, ALT, H)
// TODO: ON_DEBUG_NODE was not in OPTIM_COMP and only in 'alt_dispatcher'
#define CODE_CHOICE_NEW00(B, B0, ALT, H) do { \
  G->next_alt = (ALT); \
  (B) = (typeof(B))ChoiceNext0(B0, ChoiceArity(G)); \
  ON_DEBUG_NODE({ ((choice_t *)(B))->functor=NULL; }); \
  SetShallowTry0(((choice_t *)(B))); \
  CHPTFLG(((choice_t *)(B))->flags = 0); \
  ((choice_t *)(B))->trail_top = G->trail_top; \
  ((choice_t *)(B))->heap_top = (H); \
  w->choice = ((choice_t *)(B)); \
  NewShadowregs((H)); \
} while(0)
#define CODE_NECK_TRY(B) ({ \
  (B)->frame = G->frame; \
  (B)->next_insn = G->next_insn; \
  (B)->next_alt = G->next_alt; \
  (B)->local_top = G->local_top; \
  intmach_t arity = ChoiceArity(B); \
  for (intmach_t i = 0; i < arity; i++) { \
    (B)->x[i] = w->x[i]; \
  } \
})
// TODO: missing set event on frame overflow
#define CODE_CFRAME(Frame, NextInsn) ({ \
  (Frame)->next_insn = G->next_insn; \
  (Frame)->frame = G->frame; \
  G->frame = (Frame); \
  G->next_insn = (NextInsn); \
  G->local_top = (frame_t *)StackCharOffset((Frame),FrameSize(G->next_insn)); \
})
#define DEALLOCATE(Frame) ({ \
  G->next_insn = (Frame)->next_insn; \
  G->frame = (Frame)->frame; \
})
#define InvalidateLocalTop() G->local_top = NULL

#define CODE_MAYBE_NECK_TRY() do { \
  choice_t *b = w->choice; \
  if (IsShallowTry()) { /* try */ \
    CODE_NECK_TRY(b); \
  } \
} while(0)
#define CODE_CFRAME(Frame, NextInsn) ({ \
  (Frame)->next_insn = G->next_insn; \
  (Frame)->frame = G->frame; \
  G->frame = (Frame); \
  G->next_insn = (NextInsn); \
  G->local_top = (frame_t *)StackCharOffset((Frame),FrameSize(G->next_insn)); \
})
#define SetLocalTop(A) G->local_top = (A)

/* ------------------------------------------------------------------------- */
/* Value trail */

/* note: shares memory with the choice stack */
/* note: only usable in some built-ins, see unify and compare to check
   usage */
/* note: it is mandatory to check if enough space is available in the
   value trail before using it */

/* initial value_trail size: leave room for an extra choicept */
//#define InitialValueTrail (-ChoiceSize(0))
#define InitialValueTrail (-(ChoiceSize(0)/sizeof(tagged_t)))

/* initialize value_trail (do it one time for each worker) */
#define VALUETRAIL__INIT do { \
  w->value_trail = InitialValueTrail; \
} while(0)

/* (private) trail a value in the value trail */
#define VALUETRAIL__TRAIL(X) do { \
  tagged_t *b = (tagged_t *)w->choice; \
  intmach_t i = w->value_trail; \
  b[--i] = *(X); \
  b[--i] = (tagged_t)(X); \
  w->value_trail = i; \
} while(0)

/* set trail X and *X = Value */
#define VALUETRAIL__SET(X, Value) do { \
  VALUETRAIL__TRAIL((X)); \
  *(X) = Value; \
} while(0)

/* undo all the updates done using VALUETRAIL__SET */
#define VALUETRAIL__UNDO() do { \
  intmach_t i = w->value_trail; \
  if (i<InitialValueTrail) { \
    tagged_t *pt1; \
    tagged_t *pt2; \
    pt2 = (tagged_t *)w->choice; \
    do { \
      pt1 = (tagged_t *)pt2[i++]; \
      *pt1 = pt2[i++]; \
    } while (i<InitialValueTrail); \
    w->value_trail = (intmach_t)InitialValueTrail; \
  } \
} while(0)

// TODO:[oc-merge]: call to choice_overflow is different in OC!?
#define VALUETRAIL__TEST_OVERFLOW(AMOUNT) do { \
  if (ChoiceYounger(ChoiceCharOffset(w->choice,((AMOUNT)-w->value_trail)*sizeof(tagged_t)),w->trail_top)) { /* really: < 2*arity */ \
    CVOID__CALL(choice_overflow,2*(AMOUNT)*sizeof(tagged_t),TRUE); \
  } \
} while(0)

#define ChoiceFromChoiceTop(BT) ((choice_t *)(((char *)(BT))-w->value_trail*sizeof(tagged_t)))
#define ChoiceTopFromChoice(B) ((tagged_t *)(((char *)(B))+w->value_trail*sizeof(tagged_t)))

/* ------------------------------------------------------------------------- */
/* Events (WakeCount and interrupts) based on heap limit checks */

/* (public) */
#define TestEventOrHeapWarnOverflow(H) OffHeaptop((H),Heap_Warn_Soft)
#if defined(OPTIM_COMP)
#define TestEvent() (Heap_Warn_Soft <= Heap_Start) // TODO: Heap_Warn_Pad(CALLPAD)!=Heap_Warn_Soft instead? (unsigned integer underflow?)
#else
#define TestEvent() (Heap_Warn_Soft != Heap_Warn_Pad(CALLPAD))
#endif
#define UnsetEvent() ({ Heap_Warn_Soft = Heap_Warn_Pad(CALLPAD); })
#define TestCIntEvent() (Int_Heap_Warn == Heap_Start)
#define SetCIntEvent() ({ \
  SetEvent(); \
  Int_Heap_Warn = Heap_Start; \
})
#define UnsetCIntEvent() { Int_Heap_Warn = Heap_Warn_Pad(CALLPAD); }
/* Equivalent to: if (TestCIntEvent()) SetWakeCount(0); else UnsetEvent(); */ 
#define ResetWakeCount() ({ \
  Heap_Warn_Soft = Int_Heap_Warn; \
})
#define IncWakeCount() ({ \
  SetEvent(); \
  Heap_Warn_Soft = HeapCharOffset(Heap_Warn_Soft,-1); \
})
#define WakeCount() (TestEvent() ? HeapCharDifference(Heap_Warn_Soft,Heap_Start) : 0)
/* make WakeCount()==X (pre: TestEvent()) */
#define SetWakeCount(X) ({ \
  Heap_Warn_Soft = HeapCharOffset(Heap_Start,-(X)); \
})
/* (private) */
#define SetEvent() ({ \
  if (!TestEvent()) { \
    SetWakeCount(0); \
  } \
})
#define Int_Heap_Warn (w->int_heap_warn)
#define Heap_Warn_Soft (w->heap_warn_soft)

/* MARGINS   ------------------------------------- */

/* TODO: called when Atom_Buffer_Length is updated due to dependency
   in CALLPAD. This is not optimal! Make sure that we have enough heap
   in atom manipulation builtins instead. */

/* TODO: not needed now? (JFMC) */

/* Update heap margins (which depends on dynamic CALLPAD) */
/* TODO: changes in Int_Heap_Warn (CIntEvent) not needed now? */
#if 1 /*defined(USE_DYNAMIC_ATOM_SIZE)*/
#define UpdateHeapMargins() { \
  int wake_count = WakeCount(); \
  if (TestCIntEvent()) { \
    Int_Heap_Warn = Heap_Start; \
  } else { \
    UnsetCIntEvent(); \
  } \
  /*Heap_Warn = Heap_Warn_Pad(CALLPAD);*/ \
  if (wake_count>=0) { \
    SetWakeCount(wake_count); \
  } else { \
    ResetWakeCount(); \
  } \
}
#else
#define UpdateHeapMargins() {}
#endif

/* =========================================================================== */
/* Basic data definitions and simple macros.
 * The macros here involve casting,tagging detagging and the like term
 * to pointer conversion must know where object are in virtual memory.
 */

#include <stdio.h> /* stream_node_t requires FILE */
#include <ciao/os_signal.h> /* SIGJMP_BUF in misc_info_t */
#include <ciao/rune.h> /* for c_rune_t in stream_node */

#include <ciao/eng_alloc.h> /* alloc and declaration of flexible structures */

/* SIMPLE TYPES  & various CONSTANTS    -------------------------------   */

#if tagged__size == 32
typedef uint32_t bignum_t;
typedef int32_t signed_bignum_t;
typedef uint16_t bignum_half_t;
#define LOG2_bignum_size 5 /* 1<<5 = 32 */
#elif tagged__size == 64
typedef uint64_t bignum_t;
typedef int64_t signed_bignum_t;
typedef uint32_t bignum_half_t;
#define LOG2_bignum_size 6 /* 1<<6 = 64 */
#endif

typedef intmach_t bignum_size_t; /* size of bignums */

/* Typedefs for structs (C compiler needs them soon, since it only
   does one pass to resolve names in the source code).

   Ask me if you have any doubt (C is sometimes tricky) -- JFMC */

/* TODO: re-organize .h as in optim_comp -- JFMC */

//typedef struct worker_ worker_t;
typedef struct frame_ frame_t;
typedef struct choice_ choice_t;
typedef struct stream_node_ stream_node_t;
typedef struct goal_descriptor_ goal_descriptor_t;
typedef struct try_node_ try_node_t; /* defined in dynamic_rt.h */
typedef struct definition_ definition_t; /* defined in dynamic_rt.h */
typedef struct module_ module_t; /* defined in dynamic_rt.h */

/* ------------------------------------------------------------------------- */

/* (memory management constants for 4 reserved upper bits in pointers;
   defined dynamically by configure.c) */
#define SMALLPTR_UPPERBITS 4
#define SMALLPTR_LOWERBITS 2 /* no. of GC bits, concides with 32bit align */

#if SMALLPTR_UPPERBITS == 4
#define SMALLPTR_BASE MallocBase4
#if !(defined(USE_MMAP) && OWNMALLOC_MmapAllowed)
#define OWNMALLOC_BLOCKSIZE MIN_MEM_ALLOC_4
#endif
#define OWNMALLOC_MmapAllowed MmapAllowed4
#define OWNMALLOC_MmapSize MmapSize4
#else
#error "Unsupported SMALLPTR_UPPERBITS"
#endif

#define RelocPtr(P,Offset) ((typeof(P))((char *)(P)+(Offset)))
#define AssignRelocPtr(P,Offset) (P) = RelocPtr((P), (Offset))

// TODO: compare with OC version
#define RelocateTagged(X, FACTOR) *(X) += (FACTOR)

/* ------------------------------------------------------------------------- */
/*** tagged_t DATATYPES --------------------------------***/

#if defined(DEBUG) || defined(PROFILE)
# define DEBUG_NODE /* Adds functor information in choicepoints
                        to facilitate debugging */
#endif
/* # define DEBUG_NODE   */

#define tagged__num_offset SMALLPTR_LOWERBITS 
#define tagged__atm_offset SMALLPTR_LOWERBITS 
#define tagged__ptr_offset SMALLPTR_LOWERBITS 
#define TAGSIZE 3
#define ARITYSIZE 8
#define TAGOFFSET (tagged__size-TAGSIZE)
#define ARITYOFFSET (tagged__size-TAGSIZE-1-ARITYSIZE)
#define ARITYLIMIT (1<<ARITYSIZE) /* 256 */

#define tagged__num_size (tagged__size - TAGSIZE - 1 - tagged__num_offset)

#define TAGMASK         ((((tagged_t)1<<TAGSIZE)-1)<<TAGOFFSET) /* E000...0000 */
#define QMask           ((tagged_t)1<<TAGOFFSET>>1) /* 1000...0000 */
#define ZMask           ((tagged_t)1<<TAGOFFSET>>2) /* 0800...0000 */
 
#define INDEXMASK       (((tagged_t)1<<ARITYOFFSET)-1) /* 000F...FFFF */
#define TagIndex(T,P)   (((T)<<TAGOFFSET)+((tagged_t)((P)<<tagged__atm_offset)))
#define TagIndexDiff(P) ((tagged_t)((P)<<tagged__atm_offset))
#define IndexPart(T)    (((T)&INDEXMASK)>>tagged__atm_offset)

#define POINTERMASK     (QMask-(1<<tagged__ptr_offset)) /* 0FFF...FFFC */
#define PointerPart(T)  ((intmach_t)((T)&POINTERMASK))  
#if SMALLPTR_BASE
#define TaggedToPointer(T) ((tagged_t *)(((tagged_t)(T)&POINTERMASK)+SMALLPTR_BASE))
#else
#define TaggedToPointer(T) ((tagged_t *)((tagged_t)(T)&POINTERMASK))
#endif

/* Tagp(T,P) creates tagged_t from tag T and pointer P */
#if SMALLPTR_BASE
#define Tagp(T,P)        (((T)<<TAGOFFSET)+((tagged_t)(P) & POINTERMASK))
#else
#define Tagp(T,P)        (((T)<<TAGOFFSET)+((tagged_t)(P)))
#endif

// #define MaxAtomCount (INDEXMASK>>tagged__atm_offset)
#define MaxAtomCount (((tagged_t)1<<(tagged__size-TAGSIZE-1-ARITYSIZE-tagged__atm_offset))-1)

#define HasTag(X,T)     (((X) & TAGMASK) == ((T)<<TAGOFFSET))
#define TagOf(P)        ((P)>>TAGOFFSET)  /* collects tag */
#define CT(T1,T2)       ((T1)<<TAGSIZE|(T2)) /* for concatenating tags     */

#define IsVar(A)        ((stagged_t)(A)>=0)        /* variable tags begin with 0 */

#define TaggedIsHVA(X)     ((X) < CVA<<TAGOFFSET)
#define TaggedIsCVA(X)     HasTag(X,CVA)
#define TaggedIsSVA(X)     ((stagged_t)(X) >= (stagged_t)(SVA<<TAGOFFSET))
#define TaggedIsSmall(X)   ((stagged_t)(X) < (stagged_t)(TaggedLow+QMask))
#define TaggedIsLarge(X)   (TaggedIsSTR(X) && STRIsLarge(X))
#define TaggedIsNUM(X)     ((stagged_t)(X) < (stagged_t)(ATM<<TAGOFFSET)) 
#define TaggedIsATM(X)  HasTag(X,ATM)
#define TaggedIsLST(X)     HasTag(X,LST)
#define TaggedIsSTR(X)     ((X) >= (STR<<TAGOFFSET))

#define TaggedIsStructure(X) (TaggedIsSTR(X) && !STRIsLarge(X))
#define STRIsLarge(X)   (TaggedToHeadfunctor(X) & QMask)

/* Term<->pointer conversion */
/* NOTE: pointers must be in the SMALLPTR_BASE range and they must be
   aligned to 1<<tagged__num_offset (32-bits) */
#if SMALLPTR_BASE
#define TermToPointer(X)        ((tagged_t *)((X) ^ (TaggedZero^SMALLPTR_BASE)))
#define TermToPointerOrNull(X)  ((tagged_t *)((X)==TaggedZero ? 0 : \
                                            (X) ^ (TaggedZero^SMALLPTR_BASE)))
#define PointerToTerm(X)        ((tagged_t)(X) ^ (TaggedZero^SMALLPTR_BASE))
#define PointerToTermOrZero(X)  (!(X) ? TaggedZero : \
                                 (tagged_t)(X) ^ (TaggedZero^SMALLPTR_BASE))
#else
#define TermToPointer(X)        ((tagged_t *)((X) ^ TaggedZero))
#define TermToPointerOrNull(X)  ((tagged_t *)((X) ^ TaggedZero))
#define PointerToTerm(X)        ((tagged_t)(X) ^ TaggedZero)
#define PointerToTermOrZero(X)  ((tagged_t)(X) ^ TaggedZero)
#endif

/* Assuming IsVar(X): */
#define VarIsCVA(X)     ((stagged_t)(X<<1) >= (stagged_t)(CVA<<1<<TAGOFFSET))

/* Assuming !IsVar(X): */
#define TermIsATM(X)    ((stagged_t)(X<<1) >= (stagged_t)(ATM<<1<<TAGOFFSET))
#define TermIsLST(X)    ((stagged_t)(X<<1) < (stagged_t)(STR<<1<<TAGOFFSET))

/* Test for HVA, CVA, LST, STR i.e. 0, 1, 6, 7 (and LNUM)*/
/* This works for some machines, but not for others...
   #define IsHeapPtr(A)        ((stagged_t)(A)+(SVA<<TAGOFFSET)>=0)
*/
#define IsHeapPtr(A)   ((tagged_t)(A)+(SVA<<TAGOFFSET) < (NUM<<TAGOFFSET))

#define IsHeapVar(X)    ((X) < (SVA<<TAGOFFSET))
#define IsStackVar(X)   ((stagged_t)(X) >= (stagged_t)(SVA<<TAGOFFSET))
#define IsAtomic(X)     ((stagged_t)(X) < (stagged_t)(LST<<TAGOFFSET))
#define IsComplex(X)    ((X) >= (LST<<TAGOFFSET))

#define TermIsAtomic(X) (IsAtomic(X) || TaggedIsLarge(X))
#define TermIsComplex(X) (IsComplex(X) && !TaggedIsLarge(X))

#define TagBitFunctor  ((tagged_t)1<<TAGOFFSET)       /* ATM or STR or large NUM */
#define TagBitComplex  ((tagged_t)2<<TAGOFFSET)       /* LST or STR or large NUM */

#define TagBitCVA ((tagged_t)1<<TAGOFFSET) /* CVA (or UBV) */
#define TagBitSVA ((tagged_t)2<<TAGOFFSET) /* SVA (or UBV) */

/* If this ordering ever changes, must update other macros */

#define HVA ((tagged_t)0)               /* heap variable */
#define CVA ((tagged_t)1)               /* constrained variable */
#define SVA ((tagged_t)2)               /* stack variable */
#define UBV ((tagged_t)3)               /* Unbound -- low bits are array index */

#define NUM ((tagged_t)4)               /* number: small integer */
#define ATM ((tagged_t)5)               /* atom: low part is atmtab index */
#define LST ((tagged_t)6)               /* list */
#define STR ((tagged_t)7)               /* structure */

#define MAXTAG 7
#define NOTAG 8

#define ERRORTAG   ((tagged_t)0)        /* ERRORTAG is a tagged_t pointer guaranteed 
                                   to be different from all tagged_t objects */

/* SMall Integer Values */

/* Ranges for SMall Integer Value */
#define SmiValMax ((intval_t)(((uintval_t)1<<(tagged__num_size-1))-1))
#define SmiValMin ((intval_t)((uintval_t)(-1)<<(tagged__num_size-1)))
#define IsInSmiValRange(X) ((X) >= SmiValMin && (X) <= SmiValMax)
#if tagged__num_size == 32 /* TODO: assumes intval_t == int32_t in this case */
/* casting to int64_t because intval_t is not large enough */
#define SmiValMaxPlus1 ((int64_t)SmiValMax+1)
#define SmiValMinMinus1 ((int64_t)SmiValMin-1)
#else
#define SmiValMaxPlus1 (SmiValMax+1)
#define SmiValMinMinus1 (SmiValMin-1)
#endif

#define IntvalIsInSmiValRange(X) IsInSmiValRange(X)

/* Tags + one more bit: 
   Funny objects are represented as small ints.

   Floats and integers > 26 bits are represented as structures with
   special functors. The functors have the subtag bit = 1.
   Float=NUM, integer=ATM.

   ATM = atom as index in atmtab.
*/

#define TaggedLow       Tagp(NUM,0)
#define TaggedZero      (TaggedLow+ZMask)

#define TaggedIntMax (TaggedLow+QMask-MakeSmallDiff(1))

/* A small integer */
#define MakeSmall(X)    (((tagged_t)((intmach_t)(X)<<tagged__num_offset))+TaggedZero)
/* Get integer from small integer */
#define GetSmall(X)     ((intmach_t)(((X)>>tagged__num_offset)-(TaggedZero>>tagged__num_offset)))
/* Difference between integer and TaggedZero */  
#define MakeSmallDiff(X) ((intmach_t)(X)<<tagged__num_offset)

#define SmallAdd(U,I) ((U)+MakeSmallDiff((I)))
#define SmallSub(U,I) ((U)-MakeSmallDiff((I)))

/* Get string of an atom */
#define GetString(X)    (TaggedToAtom(X)->name)

#define ABSMACH_OPT__atom_len 1

#if defined(ABSMACH_OPT__atom_len)
#define GetAtomLen(X)   (TaggedToAtom(X)->atom_len)
#else
#define GetAtomLen(X)   (strlen(GetString((X))))
#endif

#define REALLOC_AREA(START, OLDCOUNT, NEWCOUNT) ((tagged_t *)checkrealloc_ARRAY(char, (OLDCOUNT), (NEWCOUNT), (char *)(START)))

/* 1 + no. untyped words */
#define LargeArity(X)   (PointerPart(X)>>tagged__atm_offset)
/* LargeArity() in bytes */
#define LargeSize(X)    ((PointerPart(X)>>tagged__atm_offset)*sizeof(tagged_t))

/* Pre: any tagged; Post: a functor for STR(blob(bignum)) or STR(blob(float)) */
#define BlobHF(F) ((F) & QMask)

#define BlobFunctorBignum(L) ((bignum_t)(TagIndexDiff((L))+TagIndex(ATM,1)+QMask))
#define FunctorBignumValue(T) (((T) - BlobFunctorBignum(0))>>tagged__atm_offset)

#define MakeFunctorFix   BlobFunctorBignum(1)
#define BlobFunctorFlt64 (TagIndex(NUM,3) + QMask)
#define LargeIsFloat(X)  FunctorIsFloat(TaggedToHeadfunctor(X))
#define FunctorIsFloat(X) (!((X)&TagBitFunctor))

#define MakeBlob(Ptr) make_blob(Arg,(tagged_t *)(Ptr))
#define IntmachToTagged(X) (IsInSmiValRange(X) ? MakeSmall(X) : make_integer(Arg,X))
#define IntvalToTagged(X) (IsInSmiValRange(X) ? MakeSmall(X) : make_integer(Arg,X))
#define BoxFloat(X) make_float(Arg,(X))
#define MakeAtom(X) TagIndex(ATM,X)
#define GET_ATOM(X) MakeAtom(lookup_atom_idx(X))

#define TaggedToIntmach(X) (TaggedIsSmall(X) ? GetSmall(X) : get_integer(X))
#define TaggedToFloat(X) (TaggedIsSmall(X) ? (flt64_t)GetSmall(X) : blob_to_flt64(X))

#define IsInteger(X)    (TaggedIsSmall(X) || (TaggedIsLarge(X) && !LargeIsFloat(X)))
#define IsFloat(X)      (TaggedIsLarge(X) && LargeIsFloat(X))
#define IsNumber(X)     (TaggedIsSmall(X) || TaggedIsLarge(X))
/* TODO:[oc-merge] remove IsString */
#define IsString(X)     TaggedIsATM(X)

#if BC_SCALE==2
/* SmiVal for BC32 (for BC_SCALE==2) */
#define tagged__size_BC32 32
#define tagged__num_offset_BC32 2
#define tagged__num_size_BC32 (tagged__size_BC32 - TAGSIZE - 1 - tagged__num_offset_BC32)
#define SmiValMax_BC32 ((intval_t)(((uintval_t)1<<(tagged__num_size_BC32-1))-1))
#define SmiValMin_BC32 ((intval_t)((uintval_t)(-1)<<(tagged__num_size_BC32-1)))
#define IsInSmiValRange_BC32(X) ((X) >= SmiValMin_BC32 && (X) <= SmiValMax_BC32)
#endif

/* internals.c */
flt64_t blob_to_flt64(tagged_t t);
intmach_t get_integer(tagged_t t);
CFUN__PROTO(make_float, tagged_t, flt64_t i);
/* TODO: rename to IntmachToTagged, etc. */
CFUN__PROTO(make_integer, tagged_t, intmach_t i);
CFUN__PROTO(make_blob, tagged_t, tagged_t *ptr);
CFUN__PROTO(make_structure, tagged_t, tagged_t functor);

/* X is an Integer that fits in an intmach_t.
   This is the postcondition of IntmachToTagged.
*/ 
#define IsIntegerFix(X) (TaggedIsSmall(X) || (TaggedIsSTR(X) && TaggedToHeadfunctor(X)==MakeFunctorFix))

/* TODO: backport from optim_comp */
/* size of blob, aligned to ensure correct alignment of tagged words
   in memory */
#if defined(ALIGN_BLOB_64)
#define BlobFunctorSizeAligned(X) ALIGN_TO(sizeof(int64_t), BlobFunctorSize((X)))
#define FloatFunctorSizeAligned(X) ALIGN_TO(sizeof(int64_t), FloatFunctorSize((X)))
#define BignumFunctorSizeAligned(X) ALIGN_TO(sizeof(int64_t), BignumFunctorSize((X)))
#else
/* no alignment is necessary (when disabled or when blob granularity
   matchs tagged word size */
#define BlobFunctorSizeAligned(X) BlobFunctorSize((X))
#define FloatFunctorSizeAligned(X) FloatFunctorSize((X))
#define BignumFunctorSizeAligned(X) BignumFunctorSize((X))
#endif

/* Length in bytes of unboxed data */
/* TODO:[oc-merge] LargeArity is 1+no of untyped words, must substract 1 */
#define BlobFunctorSize(X) (LargeArity((X)) * sizeof(tagged_t) - sizeof(tagged_t))

/* Create and compare large numbers from bytecode.

   In BC_SCALE==2, bignums may not be normalized, which make things a
   bit more complex. */
#if BC_SCALE == 2
CFUN__PROTO(bc_make_blob, tagged_t, tagged_t *ptr);
CBOOL__PROTO(bc_eq_blob, tagged_t t, tagged_t *ptr);
#define BC_MakeBlob(ARG, Ptr) bc_make_blob(ARG,(tagged_t *)(Ptr))
#define BC_EqBlob(T, Ptr, FailCode) {                                  \
    if (!bc_eq_blob(Arg, (T), (tagged_t *)(Ptr))) FailCode;             \
  }
#else
#define BC_MakeBlob(ARG, Ptr) make_blob(ARG,(tagged_t *)(Ptr))
#define BC_EqBlob(T, Ptr, FailCode) {                                  \
    if (!TaggedIsSTR((T))) FailCode;                                    \
    for (intmach_t i=LargeArity(*(tagged_t *)(Ptr)); i>0; i--) {                  \
      if (((tagged_t *)(Ptr))[i-1] != *TaggedToArg((T),i-1)) FailCode;  \
    }                                                                   \
  }
#endif

/* tagged_t TO POINTER and related -----------------------------------*/
/* manipulating tagged_t objects removing tag and getting correct type of
   pointer  */

#define TagpPtr(T,X) ((tagged_t*)((X)-(T<<TAGOFFSET)+SMALLPTR_BASE))

/* Functor hackery. --MC */
/*-----------------------*/

#define Arity(X)        (PointerPart(X)>>ARITYOFFSET)
#define SetArity(X,A)   ((tagged_t)(((X) & (TAGMASK | INDEXMASK)) | ((tagged_t)A<<ARITYOFFSET)))

#define TaggedToAtom(X)    (atmtab[IndexPart(X)]->value.atomp)

    /* finding the principal functor of a structure */
    /* finding the arguments of a structure, first argument is 1 */
    /* finding the car & cdr of a list. */
    /* finding the constraints of a CVA. */
#define TaggedToHeadfunctor(X) (*TagpPtr(STR,X))
#define TaggedToArg(X,N) HeapOffset(TagpPtr(STR,X),N)
#define TaggedToCar(X) TagpPtr(LST,X)
#define TaggedToCdr(X) HeapOffset(TagpPtr(LST,X),1)
#define TaggedToGoal(X) HeapOffset(TagpPtr(CVA,X),1)
#define TaggedToDef(X) HeapOffset(TagpPtr(CVA,X),2)

typedef struct instance_handle_ instance_handle_t;
typedef struct instance_ instance_t;
typedef struct int_info_ int_info_t;
typedef struct sw_on_key_ sw_on_key_t;

#define TaggedToInstance(X)        ((instance_t *)TermToPointerOrNull(X))
#define TaggedToInstHandle(X)      ((instance_handle_t *) TermToPointerOrNull(X))
#define TaggedToInstancePtr(X)     ((instance_t **)TermToPointerOrNull(X))
#define TaggedToStream(X)  ((stream_node_t *)TermToPointer(X))
#define TaggedToLock(X)    ((LOCK *)TermToPointer(X))
#define TaggedToSLock(X)   ((SLOCK *)TermToPointer(X))
#define TaggedToBool(X)    ((bool_t *)TermToPointer(X))
#define TaggedToRoot(X)    ((int_info_t *)TermToPointer(X))
#define TaggedToEmul(X)    ((emul_info_t *)TermToPointer(X))
#define TaggedToFunctor(X) ((definition_t *)TermToPointer(X))

#define TaggedToBignum(X) ((bignum_t *)TagpPtr(STR,(X)))

/* --------------------------------------------------------------------------- */

#if defined(TABLING)
typedef struct node_tr_ node_tr_t;
struct node_tr_ {
  intmach_t size;
  tagged_t *trail_sg;
  node_tr_t *next;
  node_tr_t *chain;
};
#endif

#if defined(ANDPARALLEL)

typedef struct parallel_exec_entry_ parallel_exec_entry_t;
typedef struct handler_entry_ handler_entry_t;
typedef struct event_entry_ event_entry_t;
typedef struct par_handler_ par_handler_t;

/* HANDLER structure */

struct par_handler_ {
  tagged_t goal;                       /* Parallel goal */
  bool_t det;                          /* Deterministic goal? */
  int dep_size;                        /* Dependence number size*/
  int* dep_id;                         /* Dependence number ID*/
  intmach_t exec_state;                /* Parallel execution state */
  worker_t *agent;                     /* Pointer to publishing agent */
  worker_t *remote_agent;              /* Pointer to stealing agent */
  parallel_exec_entry_t *exec_limits;  /* Limits of parallel execution */
  handler_entry_t *gle;                /* Entry into the goal list */
#if defined (VISANDOR)
  intmach_t ppf;                              /* Id */
#endif
};

/* PARALLEL EXECUTIONS LIST */
struct parallel_exec_entry_ {
  choice_t *init;               /* Pointer to beginning of goal exec */
  choice_t *end;                /* Pointer to end of goal exec */
  parallel_exec_entry_t *prev;  /* Pointer to previous one */
};

/* GOAL LIST */
struct handler_entry_ {
  tagged_t handler;              /* Handler */
  handler_entry_t *prev;  /* Pointer to previous one */
  handler_entry_t *next;  /* Pointer to next one */
};

/* EVENT QUEUE */
struct event_entry_ {
  tagged_t handler;                      /* Handler */
  bool_t canc;                           /* Cancellation or backtracking? */
  event_entry_t *prev;            /* Pointer to previous one */
  event_entry_t *next;            /* Pointer to next one */
};

/* Statistics */
typedef struct int_par_ int_par_t;
struct int_par_ {
  int value;
  int_par_t *prev;
  int_par_t *next;
};
typedef struct double_par_ double_par_t;
struct double_par_ {
  double time;
  double n;
  double_par_t *prev;
  double_par_t *next;
};

#if defined (VISANDOR)
/* EVENT for VisAndOr */
typedef struct visandor_event_ visandor_event_t;
struct visandor_event_ {
  uintmach_t timestamp;
  int evtype;
  intmach_t ppf;
  int nfork;
  worker_t *wam;
};
#endif
#endif
#if defined(PARBACK)

typedef struct goal_entry goal_entry_t;
typedef struct ans_list AL_t;
typedef struct parcall_frame PF_t;
typedef struct par_goal par_goal_t;

/* GOAL LIST */

struct goal_entry {
  par_goal_t *par_goal;              /* Handler */
  goal_entry_t *prev;  /* Pointer to previous one */
  goal_entry_t *next;  /* Pointer to next one */
};

struct ans_list {
  unsigned int numVars;
  tagged_t *vars;
  unsigned int start;
  unsigned int end;
  tagged_t *trail;
  tagged_t *pHeap;
  AL_t *next;
};

struct parcall_frame {
  PF_t *prev;                   /* Previous parallel call frame */
  SLOCK vars_l;                 /* SLOCK for combining */
  bool_t combining;             /* Some goal is combining */
  bool_t inside;                /* Some goal is combining */
//  bool_t backtracking;        /* is backtracking active? */
  unsigned int ansPending;      /* Number of goals which does not find any answer yet. */
  unsigned int failPending;     /* Number of goals which does not fail yet. */
  unsigned int numGoals;        /* Number of goals in the parcall. */
  par_goal_t *goals;            /* Goals of the parcall. */
};

/* PARALLEL GOAL structure */
struct par_goal {
  tagged_t *heapStart;
  tagged_t *localStart;
  tagged_t *trailStart;
  tagged_t goal;                /* Parallel goal */
  PF_t *pf;                     /* Parallel call frame */
  unsigned int mem_size;        /* Answers memory size */
  tagged_t *memory;             /* Memory to store answers */
  tagged_t *value_trail;        /* Value trail at time of finding the last answer (or suspension) */
  unsigned int free;            /* First free memory */
  tagged_t *array;              /* Memory to terms */
  AL_t *firstAns;               /* First produced answer */
  AL_t *nowComb;                /* Actual joined answer */
  AL_t *lastComb;               /* Last joined answer */
  AL_t *lastAns;                /* Last produced answer */
  choice_t *ch_init;            /* Initial choice point */
  choice_t *ch_end;             /* Final choice point */
  tagged_t *trail_init;         /* Initial trail pointer */
  tagged_t *trail_end;          /* Final trail pointer */
  bool_t combining;             /* is it combining its last answer? */
  intmach_t exec_state;         /* Parallel execution state */
  worker_t *agent;              /* Pointer to publishing agent */
  worker_t *remote_agent;       /* Pointer to stealing agent */
  goal_entry_t *gle;            /* Entry into the goal-cancel-back list */
  intmach_t list;                 /* Which list the handler belongs to? */
};

#endif

/* TODO: this is commonly known as 'intrusive map container based on
   doubly linked circular lists' of streams */

/* Streams: a doubly linked circular list */
struct stream_node_ {
  tagged_t label;
  stream_node_t *backward;
  stream_node_t *forward;
  tagged_t streamname;
  char streammode;
  /* pending_rune used from peek'ing. Valid values are:
   *  - a valid rune
   *  - RUNE_EOF
   *  - RUNE_VOID (indicates no pending char) */
  c_rune_t pending_rune;
  unsigned int isatty:1;
  unsigned int socket_eof:1;
  /* unsigned int socket_is_unbuffered:1; -- Not used (DCG) */
  c_rune_t previous_rune; /* To correctly count lines in Mac, Win, Unix (DCG) */
  intmach_t last_nl_pos;
  intmach_t nl_count;
  intmach_t rune_count;
  FILE *streamfile;                               /* Not used for sockets */
};

/* WAM registers */

/* There are some registers which whould be private to each worker.  Those
   which are not critical for speed are subindirected in their own blocks.
   Others I think can affect performance appear just at the worker level.
   The reason for this difference is that changing the size of the struct
   worker causes a real mess in the whole compiler, and a bootstrapping
   procedure is needed.  Thus, if new no critical registers are needed, they
   can be allocated inside of one of these blocks, and this does not change
   the size of the worker_t itself. */

typedef struct io_streams_ io_streams_t;
struct io_streams_ {
  stream_node_t *input_stream_ptr;
  stream_node_t *output_stream_ptr;
  stream_node_t *error_stream_ptr;
};

typedef struct debugger_state_ debugger_state_t;
struct debugger_state_ {
  tagged_t current_debugger_state;
  tagged_t current_debugger_mode;
};

#define USE_GLOBAL_VARS 1
#if defined(USE_GLOBAL_VARS)
#define GLOBAL_VARS_ROOT (w->misc->global_vars_root)
#endif

typedef struct misc_info_ misc_info_t;
struct misc_info_ {

/* Incore compilation of a clause sometimes requires expanding the number of
   X registers of the machine, which amounts to expanding the worker itself.
   In that case, new_worker (which is otherwise NULL) will point to the
   newly allocated worker.  This worker will be the active one upon normal
   returning to wam(). */

  worker_t *expanded_worker;

  /* From eng_gc: need to have per-worker garbage collection */

  intmach_t gc_total_grey/* = 0 */; /* accumulated upper bound of garbage left */
                                         /* Must be explicitly inited !!! */
  intmach_t gcgrey;          /* upper bound(?) of garbage left in old segments */
  intmach_t total_found;        /* the number of non-garbage words in the heap */
  tagged_t cvas_found;                 /* the last CVA found while shunting */
  choice_t *gc_aux_node;     /* aux. choicepoint for the WAM registers */
  choice_t *gc_choice_start;
  tagged_t *gc_trail_start; /* TODO: unused! */
  tagged_t *gc_heap_start;
  frame_t *gc_stack_start;
  choice_t *top_conc_chpt;  /* Topmost chicepoint for concurrent facts */
#if defined(USE_GLOBAL_VARS)
  tagged_t global_vars_root;
#endif

  /* For dynamic_neck_proceed */
  instance_t *ins; /* clause/2, instance/2 */
  
   /* For error handling through exceptions */
  int errargno;
  int errcode;
  char *errfuncname;
  int errfuncarity;
  tagged_t culprit;

  SIGJMP_BUF *errhandler;

  /* Access the goal descriptor for this thread */
  goal_descriptor_t *goal_desc_ptr;

  /* Available workers are enqueued in a list. */
  worker_t *next_worker;

  /* exit code */
  intmach_t exit_code;
  /* This goal should stop right now! */
  bool_t stop_this_goal;

#if defined(TABLING)
  //tagged_t *tabled_top = NULL;
  frame_t *stack_freg;
  tagged_t *heap_freg;
  node_tr_t *last_node_tr;
#endif

#if defined(ANDPARALLEL)
  /* TO_DO: not very efficient solution. THEY SHOULD BE ADDED IN
     (WORKER_T) DIRECTLY, but it cannot be done directly.
     SOLUTION: generate another compiler with the suitable shifts,
     then a new engine, and finally put them together. */

  /* Goal list, pointers and lock */
  handler_entry_t *goal_list_start;  /* Start of goal list */
  handler_entry_t *goal_list_top;    /* Top of goal list */
  par_handler_t *goal_cache;
  int dep_size_exec;                        /* Dependence number size*/
  int* dep_id_exec;                          /* Dependence number ID*/
  SLOCK goal_list_l;

  /* Event Queue, pointers and lock */
  event_entry_t *event_queue_start;  /* Start of event queue */
  event_entry_t *event_queue_top;    /* Top of event queue */
  SLOCK event_queue_l;

  /* Pointers to limits of goal executions */
  parallel_exec_entry_t *last_parallel_exec;

  /* Lock for creating mutual exclusions */
  SLOCK mutex_l;

  /* For expanding stacks */
  volatile int suspend;     /* Suspend the execution? */

  /* For cancellation */
  volatile bool_t cancel_goal_exec;  /* Cancel goal execution */
  volatile bool_t safe_to_cancel;    /* Can the thread safely cancel it? */

  /* Suspend until stolen goal finished or goals available */
  LOCK waiting_for_work_l;       /* Waiting for more work to do (lock) */
  COND_VAR waiting_for_work_cv;  /* Waiting for more work to do (cond_var) */
  int mode;                      /* Executing backwards or forwards */
  volatile bool_t suspended_waiting_for_work;  /* Flag */

  //#if defined(Solaris)
  ///* Measure of total suspending time of the agent */
  //double_par_t *total_suspending_time;        /* Measure */
  //hrtime_t suspending_time_cont;                     /* Measure */
  //#endif

  /* Pointer to next WAM */
  worker_t *next_wam;

#if defined(VISANDOR)
  unsigned long my_count_calls;            /* compensate clock time */
  float my_time_each_call;                 /* helps the former */
  intmach_t pcall_level;
  visandor_event_t *tmpevptr;
  visandor_event_t *nextevent;  /* current pointer into event array for this agent */
  visandor_event_t *firstevent;
  visandor_event_t *lastevent;
#endif
#endif

#if defined(PARBACK)
  /* Goal list, pointers and lock */
  goal_entry_t *goal_list_start;  /* Start of goal list */
  goal_entry_t *goal_list_top;    /* Top of goal list */
  goal_entry_t *cancel_goal_list_start;  /* Start of goal list */
  goal_entry_t *cancel_goal_list_top;    /* Top of goal list */
  goal_entry_t *back_goal_list_start;  /* Start of goal list */
  goal_entry_t *back_goal_list_top;    /* Top of goal list */
  frame_t *contFrame;

  PF_t *pf;
  
  /* Lock for creating mutual exclusions */
  SLOCK mutex_l;

  int suspend;     /* Suspend the execution? */

  /* For cancellation */
  par_goal_t *goal_to_cancel;  /* Goal to be cancelled */
  bool_t safe_to_cancel;    /* Can the thread safely cancel it? */

  /* Suspend until stolen goal finished or goals available */
  LOCK waiting_for_work_l;       /* Waiting for more work to do (lock) */
  COND_VAR waiting_for_work_cv;  /* Waiting for more work to do (cond_var) */
  bool_t suspended_waiting_for_work;  /* Flag */

  worker_t *next_wam;
#endif
};

/* States a worker can be in */
typedef enum {
  IDLE,      /* The memory areas are available for being used by a thread */
  WORKING,                 /* The memory areas are being used by a thread */
  PENDING_SOLS,               /* Frozen --  backtracking can be requested */
  FAILED                     /* Frozen -- but no more solutions available */
} Thread_State;

/* Save wam() local variables, to reenter after leaving it blocked (e.g., if
   waiting for backtracking). */

typedef struct wam_private_ wam_private_t;
struct wam_private_ {
  bcp_t p;                                             /* program counter */
  intmach_t ei; // TODO:[merge-oc] avoid saving?
  choice_t *b;
  frame_t *e;
  tagged_t *cached_r_h;
  tagged_t *r_s;
  tagged_t t0, t1, t2, t3; // TODO:[merge-oc] avoid saving?
  bcp_t ptemp; // TODO:[merge-oc] avoid saving?
};

/* Possible actions requested from the toplevel. */

#define NO_ACTION         0
#define SHARES_STRUCTURE  1 /* unused? */
#define HAS_CONTINUATION  2 /* unused? */
#define KEEP_STACKS       4
#define BACKTRACKING      8
#define CREATE_THREAD    16
#define CREATE_WAM       32
#define NEEDS_FREEING    64

/* The goal descriptors are held together in a doubly linked circular
   list; there is a pointer to the list, which points always to a free
   goal descriptor (if there is any in the list).  All the free goal
   descriptors can be found following the forward link of this initial
   pointer.  */

struct goal_descriptor_ {
  /* Pointer to the WAM registers.  
     If NULL, no WAM has been associated to the goal.  */
  worker_t *worker_registers;
  wam_private_t wam_private_state;
  /* This defines the state of the WAM (if any) associated to the goal */
  Thread_State state;      
  /* If any thread is working on the WAM, these are the points to interact
     with it */
  THREAD_ID thread_id;
  THREAD_T  thread_handle;      /* Different from thread_id in Win32 */
  int      action;              /* Defines the behavior of the goal */
  tagged_t goal;                /* The pointer to the goal to execute */
  /* NOTE: due to Term <-> int conversions, this cannot be unsigned */
  /* TODO: change type for global_goal_number? */
  intmach_t goal_number;        /* Snapshot of global counter */
  SLOCK goal_lock_l;
  goal_descriptor_t *forward, *backward;
};

struct worker_ {
  /* Space for miscelaneous stuff (or temporary hacks) */
  misc_info_t *misc;

  /* Input and output streams */
  io_streams_t *streams;

  /* Current debugger state.  Realloced into local heap. */
  debugger_state_t *debugger_info;

  /* Save info for incore compilation */
  bcp_t last_insn;

  /* Temporary allocation for various operations regarding atoms and strings. */
  char *atom_buffer;
  intmach_t atom_buffer_length;

  /* dummy */
  void *dummy0; /* TODO: experimental, used for suspended goals */
  void *dummy1;
  void *dummy2;

#if defined(USE_TAGGED_CHOICE_START)
  tagged_t *tagged_choice_start;   /* Not used any more, but I can't just
                                    remove it: the size of the WRB is
                                    critical for the compiler and changing
                                    it is a real hassle */
#else
  tagged_t *dummy3;                      /* Use up the space, therefore */
#endif    

  /* Boundaries of different areas */
  tagged_t *heap_start;
  tagged_t *heap_end;
  tagged_t *heap_warn_soft;
  tagged_t *int_heap_warn; /* Heap_Start if ^C was hit, else Heap_Warn */ // TODO: it could be a bit!
    
  tagged_t *stack_start;
  tagged_t *stack_end;
  tagged_t *dummy4; // TODO:[oc-merge] was stack_warn
    
  tagged_t *choice_end;
  tagged_t *choice_start;

  tagged_t *trail_start;
  tagged_t *trail_end;

  bcp_t liveinfo;

  choice_t *choice; /* choice pointer */
  choice_t *previous_choice; /* -""- at predicate entry */
  choice_t *segment_choice; /* gc's segment choice point */
  bcp_t insn;                   /* program counter */
  tagged_t *structure;          /* structure pointer */
  tagged_t global_uncond;               /* first uncond. global variable */
  tagged_t local_uncond;                /* first uncond. local variable no. */
  intmach_t value_trail;                /* size of value_trail extension of w->choice */

  /* incidentally, the rest is similar to a choice_t */
  try_node_t *next_alt; /* next clause at predicate entry */
  intmach_t flags;
  tagged_t *trail_top;          /* trail pointer */
  tagged_t *heap_top;         /* heap pointer */
  frame_t *frame;               /* environment pointer */
  bcp_t next_insn;              /* continuation */
  frame_t *local_top;   /* local stack pointer, or NULL if invalid */
  tagged_t x[FLEXIBLE_SIZE];         /* temporary variables */
};


struct frame_ {                 /* a.k.a. environment */
  frame_t *frame;               /* continuation frame pointer */
  bcp_t next_insn;              /* continuation program pointer */
  tagged_t x[FLEXIBLE_SIZE]; /* permanent variables */
};

typedef enum {CHOICE,MARKER} node_type;

struct choice_ {                  /* a.k.a. marker. Collapsed with a Chpt? */
/* #if defined(MARKERS) */
  /*  node_type type_of_node;*/
/* #endif */
  try_node_t *next_alt;
  intmach_t flags;
  tagged_t *trail_top;
  tagged_t *heap_top;
  frame_t *frame;
  bcp_t next_insn;
  frame_t *local_top;
#if defined(DEBUG_NODE)
  definition_t *functor;
#endif

  tagged_t x[FLEXIBLE_SIZE];            /* Offset between nodes in try_node struct */
};

#if defined(MARKERS)

typedef struct extension_regs_ extension_regs_t;
struct extension_regs_ {
  marker_t *topmost_marker;                    /* Last pushed marker */
};

typedef struct marker_ marker_t;
struct marker_ {
  node_type type_of_node;
  marker_t* previous_marker;
  marker_t* next_marker;                         /* NULL if topmost */
  worker_t saved_state;              /* Huge, but should be reduced */
};

#endif

#define USE_TRAIL_TOP_MARKS 1 // TODO:[oc-merge] disable
//#define USE_DEEP_FLAGS 1 // TODO:[oc-merge] enable
#define USE_RETRY_PATCH 1 // TODO:[oc-merge] disable

#if defined(USE_TRAIL_TOP_MARKS)
#define CHPTFLG(X) 
#define TrailTopUnmark(X) TaggedToPointer((X))
//
#define ChoiceptMarkPure(b) (*(tagged_t *)(&(b)->trail_top) |= 1)
#define ChoiceptTestPure(b) ((tagged_t)(b)->trail_top & 1)
//
#define ChoiceptMarkStatic(b) (*(tagged_t *)(&(b)->trail_top) |= 2)
#define ChoiceptTestStatic(b) ((tagged_t)(b)->trail_top & 2)
//
#define NoCVA_MARK ((tagged_t)1<<(tagged__size-1))
//
#if (SMALLPTR_BASE>>(tagged__size-1) & 0x1)
#define ChoiceptMarkNoCVA(b) (*(tagged_t *)(&(b)->trail_top) &= ~NoCVA_MARK)
#define ChoiceptTestNoCVA(b) (!((tagged_t)(b)->trail_top & NoCVA_MARK))
#else
#define ChoiceptMarkNoCVA(b) (*(tagged_t *)(&(b)->trail_top) |= NoCVA_MARK)
#define ChoiceptTestNoCVA(b) ((tagged_t)(b)->trail_top & NoCVA_MARK)
#endif
#else
#define CHPTFLG(X) X
#define TrailTopUnmark(X) ((X))
#define ChoiceptMarkPure(B) ((B)->flags |= 1)
#define ChoiceptTestPure(B) ((B)->flags & 1)
#define ChoiceptMarkStatic(B) ((B)->flags |= 2)
#define ChoiceptTestStatic(B) ((B)->flags & 2)
#define ChoiceptMarkNoCVA(B) ((B)->flags |= 4)
#define ChoiceptTestNoCVA(B) ((B)->flags & 4)
#endif

/* ------------------------------------------------------------------------- */

// TODO: document deep, shallowtry, shallowretry
//   deep: registers saved in choicept
//   shallowtry: registers not saved in choicept yet, no need to restore
//   shallowretry: registers saved in choicept, no need to restore
//   where registers are X regs, frame, next_insn, local_top

#if defined(USE_DEEP_FLAGS)
/* TODO: union for 'flags'? (different meaning than ChoiceptMarkPure, etc.) */
#define IsDeep() (G->flags == 0)
//
#define IsShallowTry() (G->flags == 1)
//
#define SetDeep0() { G->flags = 0; }
#define SetDeep() { G->flags = 0; }
#define SetShallowTry() { G->flags = 1; }
#define SetShallowTry0(B) SetShallowTry() 
#define SetShallowRetry() { G->flags = 2; }
#else
#define IsDeep() (w->next_alt == NULL)
#define IsShallowTry() (B->next_alt == NULL)
#define SetDeep0() do {} while(0)
#define SetDeep() ({ w->next_alt = NULL; })
#define SetShallowTry() SetShallowTry0(w->choice) 
#define SetShallowTry0(B) do { B->next_alt = NULL; } while(0)
#define SetShallowRetry() do {} while(0)
#endif

// TODO:[oc-merge] absmach_def:choice_patch/2
#if defined(USE_RETRY_PATCH) /* patch choice next_alt in neck retry; otherwise on failure */
#define NECK_RETRY_PATCH(B) do { B->next_alt = w->next_alt; } while(0)
#define CODE_CHOICE_PATCH(B, Alt) do { \
  G->next_alt = (Alt); \
} while(0)
#else
#define NECK_RETRY_PATCH(B) do {} while(0)
#if defined(USE_DEEP_FLAGS)
#define CODE_CHOICE_PATCH(B, Alt) do { \
  G->next_alt = (Alt); \
  (B)->next_alt = G->next_alt; \
} while(0)
#else
#define CODE_CHOICE_PATCH(B, Alt) do { \
  G->next_alt = (Alt); \
  if ((B)->next_alt != NULL) (B)->next_alt = G->next_alt; \
} while(0)
#endif
#endif

/* ------------------------------------------------------------------------- */
/* Pointers to tagged words with GC marks */

/* Deposit Source into Mask:ed portion of Dest */
#define Deposit(Source,Mask,Dest) (((Source)&(Mask))|((Dest)& ~(Mask)))

#define GC_MARKMASK  ((tagged_t)2)
#define GC_FIRSTMASK ((tagged_t)1)
#define GC_ANYMASK (GC_MARKMASK|GC_FIRSTMASK)

/* Get a tagged ensuring that it does not have gc marks */
#define GC_UNMARKED(X) ((X)&(~(GC_ANYMASK)))
#define GC_UNMARKED_M(X) ((X)&(~(GC_MARKMASK)))

#define gc_IsMarked(x)  ((x)&GC_MARKMASK)
#define gc_IsFirst(x)   ((x)&GC_FIRSTMASK)
#define gc_IsForM(x)   ((x)&(GC_FIRSTMASK|GC_MARKMASK))
#define gc_MarkM(x)  ((x)|= GC_MARKMASK)
#define gc_MarkF(x)  ((x)|= GC_FIRSTMASK)
#define gc_UnmarkM(x)  ((x)&=(~GC_MARKMASK))
#define gc_UnmarkF(x)  ((x)&=(~GC_FIRSTMASK))
#define gc_PutValue(p,x) Deposit(p,POINTERMASK,x)
#define gc_PutValueFirst(p,x) Deposit(p,POINTERMASK|GC_FIRSTMASK,x)

#if !defined(OPTIM_COMP)
#define TG_Let(X, Ptr) tagged_t *X=(Ptr); tagged_t X##val
#define TG_Val(X) X##val
#define TG_Fetch(X) ({ TG_Val(X) = *(X); })
//
#define TG_Put(V,X) ({ *(X) = (V); })
#define TG_PutPtr(p,dest) TG_Put(gc_PutValue((tagged_t)p,TG_Val(dest)), dest)
#define TG_PutPtr_SetR(curr,j) TG_Put(gc_PutValueFirst((tagged_t)curr|GC_FIRSTMASK,TG_Val(j)), j)
#define TG_PutPtr_UnsetR(dest,j) TG_Put(Deposit((tagged_t)dest,POINTERMASK|GC_FIRSTMASK,TG_Val(j)), j)

#define TG_SetR(X) gc_MarkF(*(X))
#define TG_UnsetR(X) gc_UnmarkF(*(X))

#define TG_SetM(X) gc_MarkM(*(X))
#define TG_IsM(X) gc_IsMarked(TG_Val(X))
#define TG_IsR(X) gc_IsFirst(TG_Val(X))
#define TG_IsROrM(X) gc_IsForM(TG_Val(X))
#define TG_UnsetM(X) gc_UnmarkM(*(X))
#define TG_SetAll_SetM(T, X) do { gc_MarkM(T); *(X) = (T); } while(0)

#define TG_MoveUNMARKED_M_UnsetM(src,dest) ({ \
  TG_Put(src, dest); \
})
#define TG_MoveValue_MoveR(val,curr) TG_Put(gc_PutValueFirst(val,TG_Val(curr)), curr)
#define TG_MoveValue_UnsetR(c1,A) TG_Put(Deposit(c1,POINTERMASK|GC_FIRSTMASK,TG_Val(A)), A)
#endif


#if !defined(OPTIM_COMP)
#define ASSERT__INTORC0(X, EV)
#define ASSERT__INTORC(X, EV)
#define ASSERT__VALID_TAGGED(X)
#define ASSERT__NO_MARK(X)
#endif

/* ------------------------------------------------------------------------- */
/* Alignment operations */

#define ALIGN_TO(A,X) ((((X)-1) & -(A))+(A))

/* =========================================================================== */
/* Access macros for the various WAM areas. */

#define Offset(X,O)     ((tagged_t *)(X) + (O))
#define CharOffset(X,O) ((tagged_t *)((char *)(X) + (O)))

/* THE HEAP */

/* assuming heap growth in positive direction */

#define OnHeap(t)       (HeapYounger(Heap_End,t) && !HeapYounger(Heap_Start,t))
#define OffHeaptop(t,H)          (!HeapYounger(H,t))
#define HeapYounger(X,Y)        ((tagged_t *)(X)>(tagged_t *)(Y))
#define HeapDifference(X,Y)     ((tagged_t *)(Y) - (tagged_t *)(X))
#define HeapCharDifference(X,Y) ((char *)(Y) - (char *)(X))
#define HeapOffset(X,O)         ((tagged_t *)(X) + (O))
#define HeapCharOffset(X,O)     ((tagged_t *)((char *)(X) + (O)))
#define HeapNext(X)             (*(X)++)
#define HeapPush(H,X)           (*(H) = (X), (H)++) /* X often contains H */
#define HeapDecr(H)             (--(H))

/* H: heap top */
#define HeapCharAvailable(H) HeapCharDifference((H),Heap_End)

#define HeapCharSize() HeapCharAvailable(Heap_Start)

#define Heap_Warn_Pad(PAD) HeapCharOffset(Heap_End,-(PAD))

#define HeapCharUsed(H) HeapCharDifference(Heap_Start, (H))

/* THE FRAME STACK */

/* assuming stack growth in positive direction */

#define OnStack(t) (StackYounger(Stack_End,t) && !StackYounger(Stack_Start,t))
#define OffStacktop(t,H)         (!StackYounger(H,t))
#define StackYounger(X,Y)       ((tagged_t *)(X)>(tagged_t *)(Y))
#define StackDifference(X,Y)    ((tagged_t *)(Y) - (tagged_t *)(X))
#define StackCharDifference(X,Y)        ((char *)(Y) - (char *)(X))
#define StackOffset(X,O)        ((tagged_t *)(X) + (O))
#define StackCharOffset(X,O)    ((frame_t *)((char *)(X) + (O)))
#define StackDir 1

#define StackCharAvailable(Top) StackCharDifference((Top), Stack_End)
#define StackCharUsed(Top) StackCharDifference(Stack_Start, (Top))
#define StackCharSize() StackCharAvailable(Stack_Start)

/* THE TRAIL */

/* assuming trail growth in positive direction */

#define OnTrail(t)  (TrailYounger(Trail_End,t) && !TrailYounger(Trail_Start,t))
#define TrailYounger(X,Y)       ((tagged_t *)(X)>(tagged_t *)(Y))
#define TrailDifference(X,Y)    ((tagged_t *)(Y) - (tagged_t *)(X))
#define TrailCharDifference(X,Y)        ((char *)(Y) - (char *)(X))
#define TrailOffset(X,O)        ((tagged_t *)(X) + (O))
#define TrailCharOffset(X,O)    ((tagged_t *)((char *)(X) + (O)))
#define TrailGetTop(P)          ((P)[-1])
#define TrailDec(P)             (--(P)) /* P points to the popped element */
//#define TrailPop(P)             (*--(P))
#define TrailPush(P,X)          (*(P)++ = (X))
#define TrailPushCheck(P,X)     trail_push_check(Arg,X)
#define TrailDir 1

/* THE CHOICEPOINT STACK */

/* assuming choicestack growth in negative direction */

#define OnChoice(t) (ChoiceYounger(t,Choice_Start) && !ChoiceYounger(t,Choice_End))
#define OffChoicetop(t,B)       ChoiceYounger(t,B)
#define ChoiceYounger(X,Y)      ((tagged_t *)(X)<(tagged_t *)(Y))
#define ChoiceCharOffset(X,O)   ((choice_t *)((char *)(X) - (O)))
#define ChoiceCharDifference(X,Y)       ((char *)(X) - (char *)(Y))
#define ChoiceArity(B) ((B)->next_alt->arity)
#define ChoiceCont(B)           ChoiceCont0((B), ChoiceArity((B)))
#define ChoiceCont0(B,A)        ChoiceCharOffset((B),-ChoiceSize((A)))
#define ChoiceNext0(B,A)        ChoiceCharOffset((B),ChoiceSize((A)))

#define ChoiceSize(A) SIZEOF_FLEXIBLE_STRUCT(choice_t, tagged_t, (A))

#define ChoiceCharAvailable(B) ChoiceCharDifference((B), G->trail_top)

#define ChoiceOffset(X,O)       ((tagged_t *)(X) - (O))
#define ChoiceDifference(X,Y)   ((tagged_t *)(X) - (tagged_t *)(Y))

#if defined(USE_TAGGED_CHOICE_START)
#define ChoiceFromTagged(Y) (ChoiceCharOffset(Tagged_Choice_Start,Y))
#define ChoiceToTagged(Y)   (ChoiceCharDifference(Tagged_Choice_Start,Y))
#else
#define ChoiceFromTagged(Y) ((choice_t *)ChoiceOffset(Choice_Start,(GetSmall(Y))))
#define ChoiceToTagged(Y)   (MakeSmall(ChoiceDifference(Choice_Start,Y)))
#endif

#define RefHeap(To,From) { To = *(From); }

#define RefCar(To,From) { To = *TaggedToCar(From); }

#define RefCdr(To,From) { To = *TaggedToCdr(From); }

#define RefHeapNext(To,From) { To = *(From)++; }

#define PushRefHeapNext(To,From) { *(To)++ = *(From)++; }

#define RefStack(To,From) { To = *(From); }

#define HeapPushRefStack(To,From) { *(To)++ = *(From); }

#define RefHVA(To,From) { To = *TagpPtr(HVA,From); }

#define RefSVA(To,From) { To = *TagpPtr(SVA,From); }

#define LoadSVA(Y)              {Y = Tagp(SVA,&Y); }
#define Load2SVA(X,Y)           {X = Y = Tagp(SVA,&Y); }
#define PreLoadHVA(X,H)         {X = Tagp(HVA,H); }
#define ConstrHVA(H)            {HeapPush(H,Tagp(HVA,H)); }
#define LoadHVA(To,H)           {HeapPush(H,To = Tagp(HVA,H)); }
#define Load2HVA(To1,To2,H)     {HeapPush(H,To1 = To2 = Tagp(HVA,H)); }
#define LoadCVA(To,H)           {HeapPush(H,To = Tagp(CVA,H)); }

/* =========================================================================== */
/* Definitions for symbol tables, databases, predicates. */

/* Object identifiers and emulator "pseudo instructions" - never seen
   by main dispatchers. */
typedef short enter_instr_t;

/* These six must be first, in this order. */
#define ENTER_COMPACTCODE 0
#define ENTER_COMPACTCODE_INDEXED 1
#define ENTER_PROFILEDCODE 2
#define ENTER_PROFILEDCODE_INDEXED 3
#define ENTER_FASTCODE 4
#define ENTER_FASTCODE_INDEXED 5
/* */
#define ENTER_UNDEFINED 6
#define ENTER_C 7
#define ENTER_INTERPRETED 8
#define BUILTIN_ABORT 9
#define BUILTIN_APPLY 10
#define BUILTIN_CALL 11
#define BUILTIN_SYSCALL 12
#define BUILTIN_NODEBUGCALL 13
#define BUILTIN_TRUE 14
#define BUILTIN_FAIL 15
#define BUILTIN_CURRENT_INSTANCE 16
#define BUILTIN_RESTORE 17
#define BUILTIN_COMPILE_TERM 18
#define BUILTIN_GELER 19
#define BUILTIN_INSTANCE 20
#define BUILTIN_DIF 21
#define SPYPOINT 22
#define WAITPOINT 23
#define BREAKPOINT 24
/* Other object identifiers -- these are used to inform the gc */
#define TABLE 25
#define EMUL_INFO 26
#define OTHER_STUFF 27

// uncomment to enable atom GC (incomplete)
// #define ATOMGC 1

/* OBJECT AREA ----------------------------------------------------*/ 

#define CACHE_INCREMENTAL_CLAUSE_INSERTION

typedef unsigned short int instance_clock_t;

#define ACTIVE_INSTANCE(ARG,I,TIME,CHAINP) \
  ((I)==NULL ? NULL : \
   ((TIME) >= (I)->birth && (TIME) < (I)->death) ? (I) : \
   active_instance(ARG,I,TIME,CHAINP))


 /* This one does not look at time creation of clauses. */

#define ACTIVE_INSTANCE_conc(ARG,I,ROOT)  \
     active_instance_conc(ARG,I,ROOT) 

CFUN__PROTO(active_instance, instance_t *, instance_t *i,int time,bool_t normal);

/* p->count = (intmach_t *)((char *)p + objsize) - p->counters */

#define NumberOfCounters(cl) \
  ((intmach_t *)((char *)cl + cl->objsize) - cl->counters)

/* Clauses of compiled predicates are stored as a linked list of
   records. The forward link "next" of the last clause contains the
   total number of clauses. */

typedef struct emul_info_ emul_info_t;

typedef union clause_link_ clause_link_t;
union clause_link_ {
  /* Clause pointer or number (used in emul_info to store links to
     the next clause or the total number of clauses) */
  uintmach_t number; /* clause number */
  emul_info_t *ptr;    /* clause pointer */   
};

struct emul_info_ {
  clause_link_t next;          /* next clause OR no. of clauses */
  definition_t *subdefs;
  intmach_t objsize;                             /* total # chars */
#if defined(GAUGE)
  intmach_t *counters;      /* Pointer to clause's first counter. */
#endif
  char emulcode[FLEXIBLE_SIZE];
};

/* TODO: (JFMC) give a better name to IS_CLAUSE_TAIL */

/* CLAUSE_TAIL */
#define CLAUSE_TAIL_INSNS_SIZE FTYPE_size(f_o) /* TODO: (JFMC) why? */
#define IS_CLAUSE_TAIL(EP) (EP->objsize == 0)
#define ALLOC_CLAUSE_TAIL(EP) {                                         \
    EP = checkalloc_FLEXIBLE(emul_info_t, char, CLAUSE_TAIL_INSNS_SIZE); \
    EP->subdefs = NULL;                                                 \
    EP->objsize = 0;                                                    \
  }

/* All invocations looking at an instance of an concurrent predicate will
   actually have a pointer to a pointer to the instance.  Every clause has a
   pointer to a queue of handles to itself.  Erasing a pointed to instance 
   will change the pointer to the instance itself (Confused? OK, ask me, I
   have a nice drawing on a napkin --- MCL) */

struct instance_handle_ {
  instance_t *inst_ptr;                    /* Pointer to the instance */
  tagged_t head;                            /* Goal called; allows indexing. */
  instance_handle_t *next_handle;             
  instance_handle_t *previous_handle;             
};

/* Terms recorded under a key or clauses of an interpreted predicate
   are stored as a doubly linked list of records.  The forward link is
   terminated by NULL; the backward link wraps around.  Each instance
   points back to the beginning of the list.  The rank field defines a
   total order on a list.  Two timestamps are associated with each
   instance to support proper semantics for dynamic code
   updates. (MCL, with help from the manual).  There are also two
   pointers (one for unindexed and other for indexed accesses) to
   queues which maintain the list of calls looking at each
   instance. */

struct instance_ {
  instance_t *forward;
  instance_t *backward;
  int_info_t *root;
  instance_t *next_forward;
  instance_t *next_backward;
  tagged_t key;
  tagged_t rank;
  instance_clock_t birth, death;                          /* Dynamic clause lifespan */

  instance_handle_t *pending_x2;       /* Seen by invocations looking @ here */
  instance_handle_t *pending_x5;       /* Seen by invocations looking @ here */

  int objsize;                                           /* total # chars */
  char emulcode[FLEXIBLE_SIZE];
};

/* Indexing information for dynamic predicates? First simple, common cases,
   then hash table indexing.  Includes information on openness/closeness of
   concurrent facts. (MCL) */

typedef enum {DYNAMIC, CONC_OPEN, CONC_CLOSED} int_behavior_t;

struct int_info_ {
  volatile int_behavior_t  behavior_on_failure;/* behavior if no clauses match. */

#if defined(CONDITIONAL_VARS)
  condition_t clause_insertion_cond;
#else
  /*  SLOCK clause_insertion_cond;            */
  condition_t clause_insertion_cond;
#endif

  instance_handle_t *x2_pending_on_instance;     /* Used when pred. is empty */
  instance_handle_t *x5_pending_on_instance;

  instance_t  *first  ;
  instance_t  *varcase;
  instance_t  *lstcase;
  sw_on_key_t *indexer;
};

typedef struct und_info_ und_info_t;
struct und_info_ {
  int unused;
};

extern stream_node_t *root_stream_ptr;

 /* Information about atoms */

typedef struct atom_ atom_t;
struct atom_ {        
  unsigned int has_squote:1;
  unsigned int has_dquote:1;
  unsigned int has_special:1;
  unsigned int index:29;
                               /* support for locking on atom names. MCL. */
#if defined(USE_THREADS)                    /* Do not waste space otherwise */
  LOCK atom_lock_l;                      /* May be held for a long time */
#if defined(ABSMACH_OPT__general_locks)
  volatile intmach_t atom_lock_counter;         /* For general semaphores */
  SLOCK counter_lock;                            /* Held for a short time */
#endif
#endif
  uintmach_t atom_len;
  char name[FLEXIBLE_SIZE];
};

/* For a given goal and an emulated predicate, alternatives that might match
   the goal are stored as a linked list of records */

/* try-retry-trust repr. as a linked list. */

struct try_node_ {
  try_node_t *next;                      /* Next alternative or NULL */
  bcp_t emul_p;                    /* write mode or not first alternative */
  bcp_t emul_p2;                          /* read mode, first alternative */
  intmach_t arity;                 /* arity of this choicepoint */
  /*short number;*/
  uintmach_t number;                /* clause number for this alternative */
                             /* Gauge specific fields MUST come after this*/
#if defined(GAUGE)
  intmach_t *entry_counter;        /* Offset of counter for clause entry */
#endif
};

#define SwitchSize(X) (((X)->mask / sizeof(sw_on_key_node_t))+1) 
#define SizeToMask(X) (((X)-1) * sizeof(sw_on_key_node_t))

typedef struct sw_on_key_node_ sw_on_key_node_t;
struct sw_on_key_node_ {
  /* NOTE: Make sure that the size of this structure is a power of 2
     (for efficiency) */
  tagged_t key;
  union {
    try_node_t *try_chain;         /* try-retry-trust as linked list */
    instance_t *instp;             /* int. clauses or recorded terms */
    sw_on_key_node_t *node;
    definition_t *def;                       /* predicate definition */
    module_t *mod;                              /* module definition */
    int_info_t *irootp;                             /* indexer info? */
    atom_t *atomp;               /* Info on atoms and main functors  */
    void *proc;                /* C function pointer (CFUN or CBOOL) */
  } value;
};

/* Indexing table are used in indexing on first argument in calls and in
   looking up predicates.  They are operated as hash tables with quadratic
   overflow handling.  Hash table access is performed by using some of the
   low order bits of the key as array index, and then searching for a hit or
   for a zero key indicating that the key is absent from the table. 
   
   MCL: changed to make room for erased atoms: now a key == 1 indicates
   that the entry has been erased (but the search chain continues).  New 
   atoms are placed in the first free entry.

   NOTE (MCL): data structure different from that in Sicstus 1.8!!
*/

struct sw_on_key_ {
  uintmach_t mask;                                          /* bitmask */
  intmach_t count;
#if defined(ATOMGC)
  intmach_t next_index;
#endif
  sw_on_key_node_t node[FLEXIBLE_SIZE];
};

/* Node from a byte offset */
#define SW_ON_KEY_NODE_FROM_OFFSET(Tab, Offset) \
  ((sw_on_key_node_t *)((char *)&(Tab)->node[0] + (Offset)))

typedef struct incore_info_ incore_info_t;
struct incore_info_ {
  clause_link_t clauses;                                  /* first clause */
  /* Pointer to either "next" field of last clause or "clauses"  */
  clause_link_t *clauses_tail;
#if defined(CACHE_INCREMENTAL_CLAUSE_INSERTION)
  emul_info_t *last_inserted_clause;        /* Pointer to the last clause */
  uintmach_t last_inserted_num;           /* Number of last ins. clause */
#endif
  try_node_t *varcase;
  try_node_t *lstcase;
  sw_on_key_t *othercase;
};

#define SetEnterInstr(F,I) \
{ \
  (F)->predtyp = (I); \
  (F)->enter_instr = \
    (F)->properties.spy ? SPYPOINT : \
      (F)->properties.wait ? WAITPOINT : \
        (F)->properties.breakp ? BREAKPOINT : \
          (F)->predtyp; \
}

typedef union definfo_ definfo_t;
union definfo_ {
  int_info_t *intinfo;
  und_info_t *undinfo;
  incore_info_t *incoreinfo;
  void *proc;
};

struct definition_ {
  enter_instr_t enter_instr;
  short arity; /*  */
  tagged_t printname;                           /* or sibling pointer | 1 */
                                                /* or parent pointer | 3 */
  struct {
    unsigned int spy:1;
    unsigned int breakp:1;
    /* unsigned int public:1; */
                      /* concurrent obeys declaration and implies dynamic */
    unsigned int concurrent:1;      /* 1 if wait on it, 0 if closed (MCL) */
    unsigned int wait:1;                             /* obeys declaration */
    unsigned int dynamic:1;                                       /* -""- */
    unsigned int multifile:1;                                     /* -""- */
    unsigned int nonvar:1;             /* seen P(X,...) :- var(X), !, ... */
    unsigned int var:1;             /* seen P(X,...) :- nonvar(X), !, ... */
  } properties;
  enter_instr_t predtyp;
  definfo_t code;
};

#define DEF_SIBLING(F) \
  ((F)->printname&2 ? NULL : (definition_t *)TaggedToPointer((F)->printname))

struct module_ {
  tagged_t printname;
  struct {
    unsigned int is_static:1;
  } properties;
};

/* Classified somewhere else */
extern sw_on_key_node_t **atmtab;
extern sw_on_key_t *ciao_atoms;
extern void *builtintab[];

typedef struct statistics_ statistics_t;
struct statistics_ {
  inttime_t ss_tick;                         /* time spent stack_shifting */
  intmach_t ss_global;                                 /* # global shifts */
  intmach_t ss_local;                                  /* # local shifts  */
  intmach_t ss_control;                         /* # control/trail shifts */
  inttime_t gc_tick;                             /* Total GC ticks (sec) */
  intmach_t gc_count;                            /* # garbage collections */
  intmach_t gc_acc;                         /* Total reclaimed heap space */

  inttime_t starttick;
  inttime_t lasttick;

  inttime_t startwalltick;
  inttime_t lastwalltick;
  inttime_t wallclockfreq;

  inttime_t startusertick;
  inttime_t lastusertick;
  inttime_t userclockfreq;

  inttime_t startsystemtick;
  inttime_t lastsystemtick;
  inttime_t systemclockfreq;
};

extern statistics_t ciao_stats; /* Shared, I guess */

#if defined(GAUGE)
#define INCR_COUNTER(c)   ((*(c))++)
#endif

typedef struct other_stuff_ other_stuff_t;
struct other_stuff_ {     /* Usable type for passing data areas to gc etc */
  char *pointer;
  intmach_t size;
};

/* =========================================================================== */

CBOOL__PROTO(cunify, tagged_t x1, tagged_t x2);
CBOOL__PROTO(cunify_args, int arity, tagged_t *pt1, tagged_t *pt2);

#define CBOOL__UNIFY(X, Y) CBOOL__CALL(cunify, (X), (Y))
#define CBOOL__LASTUNIFY(X, Y) CBOOL__LASTCALL(cunify, (X), (Y))

/* =========================================================================== */

#include <ciao/eng_debug.h>

/* =========================================================================== */
/* Global definitions for locks, terms, predicates, predicate tables,
   workers, etc. */
 
/*  INITIALIZATION INFO 
    definitions, structures, and global variables needed for
    initializations */

/* These should be local to a thread */

/* extern tagged_t heap_start, *heap_end, heap_warn, *heap_warn_soft,
   *stack_start, *stack_end, *stack_warn, tagged_choice_start, choice_start,
   *choice_end, trail_start, *trail_end; */

/* extern char *atom_buffer; */  /* Non-shared --- used by each worker */


/* I believe that atom_buffer_length can change dynamically; should be
   private to each worker, then.  Will that pose problems problems with the
   hashing functions? */

/* extern int atom_buffer_length; */  /* Non-shared --- used by each worker */


extern sw_on_key_t *prolog_predicates;    /* Shared -- never changes */
extern sw_on_key_t **predicates_location;                  /* Shared */
extern sw_on_key_t *prolog_modules;    /* Shared -- never changes */
extern sw_on_key_t **modules_location;                  /* Shared */

/* Database locks */
extern SLOCK    prolog_predicates_l;                      /* Pointer to it */
extern SLOCK    prolog_modules_l;                         /* Pointer to it */

/* Wait until new worker Id is generated */
extern SLOCK    worker_id_pool_l;
extern SLOCK    atom_id_l;
extern SLOCK    wam_list_l;

#if defined(PARBACK)
extern SLOCK wam_circular_list_l;   /* Mutex for adding stack sets into list */

extern worker_t *main_worker;  /* Pointer to main worker */

extern volatile int nagents;      /* Number of agents created */
extern SLOCK nagents_l;
#endif

#if defined(ANDPARALLEL)
extern SLOCK wam_circular_list_l;   /* Mutex for adding stack sets into list */
extern SLOCK stackset_expansion_l;  /* Mutex for stack set expansion */

extern worker_t *main_worker;  /* Pointer to main worker */
extern bool_t unwinding_done;         /* Has the unwinding been performed? */

extern int nagents;      /* Number of agents created */
extern SLOCK nagents_l;

/* Parallel goals */
extern int npargoals;
/* Parallel goals stolen */
extern int_par_t *npargoalstaken;
/* Backw exec over par local goal */
extern int_par_t *nlocalbacktr;
/* Backw exec over par rem top goal */
extern int_par_t *nrembacktr_top;
/* Backw exec over par rem trp goal */
extern int_par_t *nrembacktr_trapped;

extern SLOCK npargoals_l;
extern SLOCK npargoalstaken_l;
extern SLOCK nlocalbacktr_l;
extern SLOCK nrembacktr_top_l;
extern SLOCK nrembacktr_trapped_l;

extern bool_t measure;
#endif

#if defined(ANDPARALLEL) && defined(VISANDOR)
/* Event Tracing Flags etc for VisAndOr */
extern int nacagents;    /* Number of active agents */
extern int maxevents;    /* maximum number of events */
extern bool_t gen_event_file;
extern float time_at_event_start;
#endif

#if defined(DEBUG)
extern SLOCK    ops_counter_l;
#endif


extern bool_t wam_initialized;/* Non-shared? --- set by each worker to decide whether re-initialize or exit after a failure */

extern char *default_ciaoroot;
extern char *default_c_headers_dir;

 /* All atom & functor definitions are shared */

#if defined(MARKERS)
extern tagged_t atom_success;
extern tagged_t atom_failure;
#endif

extern tagged_t atom_share;
extern tagged_t atom_noshare;
extern tagged_t atom_nil;
extern tagged_t atom_list;
extern tagged_t atom_read;
extern tagged_t atom_write;
extern tagged_t atom_append;
extern tagged_t atom_socket;
extern tagged_t atom_symlink;
extern tagged_t atom_regular;
extern tagged_t atom_directory;
extern tagged_t atom_fifo;
extern tagged_t atom_stdout;
extern tagged_t atom_unknown;
extern tagged_t atom_prolog;
extern tagged_t atom_lessthan;
extern tagged_t atom_greaterthan;
extern tagged_t atom_equal;
extern tagged_t atom_off;
extern tagged_t atom_on;
extern tagged_t atom_error;
extern tagged_t atom_trace;
extern tagged_t atom_debug;
extern tagged_t atom_fail;
extern tagged_t atom_all;
extern tagged_t atom_terse;
extern tagged_t atom_verbose;
extern tagged_t atom_compiled;
extern tagged_t atom_interpreted;
extern tagged_t atom_builtin;
extern tagged_t atom_true;
extern tagged_t atom_false;
extern tagged_t atom_retry_hook;
extern tagged_t atom_unprofiled;
extern tagged_t atom_profiled;
/*extern tagged_t atom_public;*/
extern tagged_t atom_wait;
extern tagged_t atom_dynamic;
extern tagged_t atom_concurrent;
extern tagged_t atom_multifile;
extern tagged_t atom_user;
extern tagged_t atom_att;

extern tagged_t atom_self;
extern tagged_t atom_create;

extern tagged_t atom_default_ciaoroot;
extern tagged_t atom_default_c_headers_dir;

extern tagged_t atom_block;
extern tagged_t atom_no_block;

#if defined(GAUGE)
extern tagged_t atom_counter;
#endif

extern tagged_t functor_neck;
extern tagged_t functor_list;
extern tagged_t functor_cut;
extern tagged_t functor_minus;
extern tagged_t functor_slash;
extern tagged_t functor_and;
extern tagged_t functor_functor;
extern tagged_t functor_tagged;
extern tagged_t functor_emul_entry;
extern tagged_t functor_builtin;
extern tagged_t functor_Dref;
extern tagged_t functor_Dstream;
extern tagged_t functor_Dlock;
extern tagged_t functor_Dhandler;
extern tagged_t functor_Dsetarg;
extern tagged_t functor_large;
extern tagged_t functor_long;

extern tagged_t functor_active;
extern tagged_t functor_pending;
extern tagged_t functor_failed;
extern tagged_t functor_available;

extern tagged_t current_prompt;
extern tagged_t current_unknown;
/* extern tagged_t current_leash_mode; */
/* extern tagged_t current_maxdepth; */
/* extern tagged_t current_printdepth; */
/* extern tagged_t current_breaklevel; */
extern tagged_t current_compiling;
extern tagged_t current_ferror_flag;
/* extern tagged_t current_discontiguous_flag; */
extern tagged_t current_quiet_flag;
/*extern tagged_t current_debugger_state;*/  /* Now private */
/*extern tagged_t current_debugger_mode;*/   /* Now private */
extern tagged_t current_radix;

extern try_node_t *address_nd_repeat;
extern try_node_t *address_nd_current_instance;
extern try_node_t *address_nd_current_atom;
extern try_node_t *address_nd_current_predicate;
extern try_node_t *address_nd_predicate_property;
extern try_node_t *address_nd_current_stream;
#if defined(TABLING)
extern try_node_t *address_nd_fake_choicept;
#endif
#if defined(PARBACK)
extern try_node_t *address_nd_suspension_point;
extern bcp_t restart_point_insn;
#endif
extern try_node_t *address_nd_yield;

extern definition_t *address_true;
extern definition_t *address_fail;

extern definition_t *address_call;
// #if defined(INTERNAL_CALLING)
// extern definition_t *address_internal_call;
// #endif
extern definition_t *address_interpret_goal;
extern definition_t *address_interpret_compiled_goal;
extern definition_t *address_interpret_c_goal;
extern definition_t *address_undefined_goal;
extern definition_t *address_help; 
extern definition_t *address_restart; 
extern definition_t *address_trace;
extern definition_t *address_getct;
extern definition_t *address_getct1;
extern definition_t *address_get;
extern definition_t *address_get2;
extern definition_t *address_get1;
extern definition_t *address_get12;
extern definition_t *address_peek;
extern definition_t *address_peek2;
extern definition_t *address_get_byte1;
extern definition_t *address_get_byte2;
extern definition_t *address_peek_byte1;
extern definition_t *address_peek_byte2;
extern definition_t *address_skip;
extern definition_t *address_skip2;
extern definition_t *address_skip_line;
extern definition_t *address_skip_line1;
extern definition_t *address_error;

CBOOL__PROTO(nd_repeat);
CBOOL__PROTO(nd_current_atom);
CBOOL__PROTO(nd_current_clauses);
CBOOL__PROTO(nd_current_predicate);
CBOOL__PROTO(nd_predicate_property);
CBOOL__PROTO(nd_current_stream);
#if defined(TABLING)
CBOOL__PROTO(nd_fake_choicept);
#endif
#if defined(PARBACK)
CBOOL__PROTO(nd_suspension_point);
#endif

definition_t *define_c_mod_predicate(char *module, char  *pname, int arity, cbool0_t procedure);
void undefine_c_mod_predicate(char *module, char *pname, int arity); 

module_t *define_c_static_mod(char *module_name);

/* =========================================================================== */
/* Term deref and switch */

#define DerefHeap(Xderef,Ptr) \
{ \
  tagged_t m_i; \
  tagged_t m_j; \
 \
  RefHeap(m_i,Ptr); \
  DerefHeapSwitch(m_i,m_j,{break;}) \
  Xderef = m_i; \
}

#define DerefCar(Xderef,Ptr) \
{ \
  tagged_t m_i; \
  tagged_t m_j; \
 \
  RefCar(m_i,Ptr); \
  DerefHeapSwitch(m_i,m_j,{break;}) \
  Xderef = m_i; \
}

#define DerefCdr(Xderef,Ptr) \
{ \
  tagged_t m_i; \
  tagged_t m_j; \
 \
  RefCdr(m_i,Ptr); \
  DerefHeapSwitch(m_i,m_j,{break;}) \
  Xderef = m_i; \
}

#define DerefArg(Xderef,Ptr,I) \
{ \
  tagged_t m_i; \
  tagged_t m_j; \
 \
  m_i = *TaggedToArg(Ptr,I); \
  DerefHeapSwitch(m_i,m_j,{break;}) \
  Xderef = m_i; \
}

#define DerefHeapNext(Xderef,Ptr) \
{ \
  tagged_t m_i; \
  tagged_t m_j; \
 \
  RefHeapNext(m_i,Ptr); \
  DerefHeapSwitch(m_i,m_j,{break;}) \
  Xderef = m_i; \
}


#define DEREF(Xderef,X) \
{ \
  tagged_t m_i; \
  tagged_t m_j; \
 \
  m_i = X; \
  DerefSwitch(m_i,m_j,;) \
  Xderef = m_i; \
}

#define DerefSw_HVAorCVAorSVA_Other(Reg,CODE_HVAorCVAorSVA,CODE_Other) ({ \
  __label__ labelend; \
  if (IsVar(Reg)) { \
    for(;;) { \
      tagged_t Aux; \
      Aux = *TaggedToPointer(Reg); \
      if (Reg == Aux) { \
        CODE_HVAorCVAorSVA; \
        goto labelend; \
      } \
      Reg = Aux; \
      if (!IsVar(Reg)) break; \
    } \
  } \
  CODE_Other; \
labelend: {} \
})

#define DerefSw_CVA_Other(X, CODE_CVA, CODE_Other) { \
  __label__ derefsw_cva; \
  __label__ derefsw_other; \
  __label__ derefsw_end; \
  tagged_t t_; \
  DerefSwitch((X),t_,{ if (VarIsCVA((X))) goto derefsw_cva; }); \
  goto derefsw_other; \
 derefsw_cva: \
  CODE_CVA; \
  goto derefsw_end; \
 derefsw_other: \
  CODE_Other; \
  goto derefsw_end; \
 derefsw_end: {} \
}

//TODO:[merge-oc] DerefSw_HVA_CVA_SVA_Other?
#define SwitchOnVar(Reg,Aux,HVACode,CVACode,SVACode,NVACode) \
{ \
    for (;;) \
      { \
          if (!IsVar(Reg)) \
            NVACode \
          else if (Reg & TagBitSVA) \
            { RefSVA(Aux,Reg); \
              if (Reg!=Aux) { Reg=Aux; continue; } \
              else SVACode \
            } \
          else if (!(Reg & TagBitCVA)) \
            { RefHVA(Aux,Reg); \
              if (Reg!=Aux) { Reg=Aux; continue; } \
              else HVACode \
            } \
          else \
            { Aux = *TagpPtr(CVA,(Reg)); \
              if (Reg!=Aux) { Reg=Aux; continue; } \
              else CVACode \
            } \
          break; \
        } \
}

#define SwitchOnHeapVar(Reg,Aux,HVACode,CVACode,NVACode) \
{ \
    for (;;) \
      { \
          if (!IsVar(Reg)) \
            NVACode \
          else if (!(Reg & TagBitCVA)) \
            { RefHVA(Aux,Reg); \
              if (Reg!=Aux) { Reg=Aux; continue; } \
              else HVACode \
            } \
          else \
            { Aux = *TagpPtr(CVA,(Reg)); \
              if (Reg!=Aux) { Reg=Aux; continue; } \
              else CVACode \
            } \
          break; \
        } \
}

#define Sw_HVA_CVA_SVA_Other(T, HVACode, CVACode, SVACode, NONVARCode) do { \
  if (!IsVar((T))) { \
    NONVARCode; \
  } else { \
    if ((T) & TagBitSVA) { \
      SVACode; \
    } else if (!((T) & TagBitCVA)) { \
      HVACode; \
    } else { \
      CVACode; \
    } \
  } \
} while(0);

#define DerefSwitch(Reg,Aux,VarCode) \
{ \
  if (IsVar(Reg)) \
    do \
      if (Reg == (Aux = *TaggedToPointer(Reg))) \
        {VarCode;break;} \
    while (IsVar(Reg=Aux)); \
}

#define DerefHeapSwitch(Reg,Aux,VarCode) DerefSwitch(Reg,Aux,VarCode)

#define SwEval(V, HeadFunctor, NUMCode, LSTCode, BlobCode, STRCode, OtherCode) ({ \
  switch (TagOf((V))) { \
  case NUM: NUMCode; break; \
  case LST: LSTCode; break; \
  case STR: \
    if (STRIsLarge((V))) { \
      BlobCode; \
    } else { \
      tagged_t HeadFunctor = TaggedToHeadfunctor((V)); \
      STRCode; \
    } \
    break; \
  default: \
    OtherCode; break; \
  } \
})

#define YoungerHeapVar(Q,R)     HeapYounger(Q,R)
#define YoungerStackVar(Q,R)    StackYounger(Q,R)

#if defined(PARBACK) || defined(ANDPARALLEL)
#define CondHVA(X)              (!OffHeaptop(X,w->global_uncond) || !OnHeap(TaggedToPointer(X)))
#define CondCVA(X)              (!OffHeaptop(Tagp(HVA,TagpPtr(CVA,X)),w->global_uncond) || !OnHeap(TaggedToPointer(X)))
#define CondSVA(X)              (!OffStacktop(X,w->local_uncond) || !OnStack(TaggedToPointer(X)))
#else
#define CondHVA(X)              (!OffHeaptop(X,w->global_uncond))
#define CondCVA(X)              (!OffHeaptop(Tagp(HVA,TagpPtr(CVA,X)),w->global_uncond))
#define CondSVA(X)              (!OffStacktop(X,w->local_uncond))
#endif
#define CondStackvar(X)         CondSVA(X)

/* segfault patch -- jf */
CVOID__PROTO(trail_push_check, tagged_t x);

/* segfault patch -- jf */
#define BindCVANoWake(U,V) \
{ \
  TrailPushCheck(w->trail_top,U); \
  *TagpPtr(CVA,U) = V; \
}

#define BindCVA(U,V)                            \
  {                                             \
    IncWakeCount();                             \
    TrailPushCheck(w->trail_top,U);             \
    *TagpPtr(CVA,U) = V;                           \
  }

#define BindSVA(U,V)                            \
  {                                             \
    if (CondSVA(U))                             \
      TrailPushCheck(w->trail_top,U);           \
    *TagpPtr(SVA,U) = V;                           \
  }

#define BindHVA(U,V)                            \
  {                                             \
    if (CondHVA(U))                             \
      TrailPushCheck(w->trail_top,U);           \
    *TagpPtr(HVA,U) = V;                           \
  }

#define NULL_TRAIL_ENTRY MakeSmall(0)
#define IsCanceled(T) (T == NULL_TRAIL_ENTRY)
#define NullifyTrailEntry(P) *(P) = NULL_TRAIL_ENTRY

//TODO: nullify fake trail entries with a predicate which makes nothing.
#define PlainUntrail(TR,Ref,CONT)                                       \
  {                                                                     \
    TrailDec(TR);                                                       \
    Ref = *(TR); /* (tr points to the popped element) */                \
    if (!IsVar(Ref))                                                    \
      {if (!IsCanceled(Ref)) CONT}                                      \
    else                                                                \
      *TaggedToPointer(Ref) = Ref;                                         \
  } 

#define COMPRESS_TRAIL(CP, CURR, DEST) do { \
  tagged_t *limit; \
  limit = TrailTopUnmark((CP)->trail_top); \
  while (TrailYounger(limit,(CURR))) { \
    tagged_t cv; \
    cv = *(CURR); \
    (CURR)++; \
    if (cv != (tagged_t)0) { \
      TrailPush((DEST),cv); \
    } \
  } \
  (CP)->trail_top -= limit-(DEST); \
} while(0);

#define CompressTrailNoGC(tr0) ({ \
  tagged_t *h; \
  tagged_t *tr; \
  tr = tr0; \
  h = tr; \
  COMPRESS_TRAIL_NOGC(G, tr, h); \
})
#define COMPRESS_TRAIL_NOGC(CP, SRC, DEST) ({ \
  tagged_t *limit = (CP)->trail_top; \
  while (TrailYounger(limit, (SRC))) { \
    tagged_t ref = *(SRC); \
    (SRC)++; \
    if (ref != (tagged_t)0) { \
      TrailPush(DEST,ref); \
    } \
  } \
  (CP)->trail_top = (DEST); \
})

/* SERIOUS_FAULT - a fault that should not occur- indicating a corruption
                  such as following the STR tag not coming to a FNT tag
                  this kind of fault may not need to be testing in final
                  version but must in testing cause a total abort
   USAGE_FAULT   - a fault in the usage(incorrect parameters) of a 
                  builtin predicate - an error message is written.
   MINOR_FAULT   - a fault that should result in a error message being
                  written somewhere, but the builtin predicate just
                  fails and is not aborted
*/

/* --------------------------------------------------------------------------- */
/* support for copy term */

#define TopOfOldHeap TagpPtr(HVA,w->global_uncond)

/* Enable copying of terms between different heaps in copy_term */
/* (it does not have any significant impact on performance) */
#define SAFE_CROSS_COPY 1

/* assert(HVA==0) */
/* Detect if a variable is old during copy_term */
#if defined(SAFE_CROSS_COPY)
/* Use TopOfOldHeap as a memory barrier and OnHeap (for safe cross
   copy_term, since we cannot not assume anything about the order of
   different heaps */
#define OldHVA(X) (!OffHeaptop(X,w->global_uncond) || !OnHeap(TaggedToPointer(X)))
#define OldCVA(X) (!OffHeaptop(Tagp(HVA,TagpPtr(CVA,X)),w->global_uncond) || !OnHeap(TaggedToPointer(X)))
#else
/* Use TopOfOldHeap as a memory barrier */
#define OldHVA(X) (!OffHeaptop(X,w->global_uncond))
#define OldCVA(X) (!OffHeaptop(Tagp(HVA,TagpPtr(CVA,X)),w->global_uncond))
#endif

/* =========================================================================== */
/* Faults and errors */

/* Exit code from wam() when aborting */
#define WAM_ABORT -32768 /* see exceptions.pl */
#define WAM_INTERRUPTED -32767

#include <setjmp.h>
#include <ciao/os_signal.h>

extern SIGJMP_BUF abort_env;

void failc(char *mesg);

#define PANIC_FAULT SERIOUS_FAULT /* TODO: use oc panic fault? */

#define SERIOUS_FAULT(Y) { \
    failc(Y); \
    SIGLONGJMP(abort_env, WAM_ABORT); \
  }
                          
#define MAJOR_FAULT(Y) { failc(Y); return FALSE; }

#define USAGE_FAULT(Y) { failc(Y); return FALSE; }

#define MINOR_FAULT(Y) { return FALSE; }

/* =========================================================================== */
/* Error codes, xref errhandle.pl, internals.pl //) */

/* OGRAMA: error classification ISO PROLOG */
/* JFMC: renamed macros */

/* For any change / addition to this list:
   
            PLEASE DO READ AND UPDATE THE CORRESPONDING FACTS IN
                        core/engine/internals.pl

    Error messages are given by core/lib/errhandle.pl .  Learn how these
    work before updating anything here.
*/
/* TODO:[oc-merge] generate automatically, update */

/* Errors identifiers cannot be zero (as 0 = -0) */
#define INSTANTIATION_ERROR     1
#define UNINSTANTIATION_ERROR   2
#define TYPE_ERROR(D)           (RANGE_PER_ERROR*error_start(type)+D)
#define DOMAIN_ERROR(D)         (RANGE_PER_ERROR*error_start(dom)+D)
#define EXISTENCE_ERROR(D)      (RANGE_PER_ERROR*error_start(exist)+D)
#define PERMISSION_ERROR(D,F)   (RANGE_PER_ERROR*error_start(perm)+D*10+F)
#define REPRESENTATION_ERROR(D) (RANGE_PER_ERROR*error_start(repres)+D)
#define EVALUATION_ERROR(D)     (RANGE_PER_ERROR*error_start(eval)+D)
#define RESOURCE_ERROR(D)       (RANGE_PER_ERROR*error_start(res)+D)
#define SYNTAX_ERROR            (RANGE_PER_ERROR*error_start(syntax))
#define SYSTEM_ERROR            (RANGE_PER_ERROR*error_start(system))
/* TODO:[oc-merge] add FOREIGN_ERROR in optim-comp */
#define FOREIGN_ERROR           (RANGE_PER_ERROR*error_start(foreign))
#define USER_EXCEPTION          (RANGE_PER_ERROR*error_start(user))

#define RANGE_PER_ERROR 100                    /* Enough number of errors */
#define error_start(KEY) error_start__##KEY
#define error_start__inst    0
#define error_start__type    1
#define error_start__dom     2
#define error_start__exist   3
#define error_start__perm    4
#define error_start__repres  5
#define error_start__eval    6
#define error_start__res     7
#define error_start__syntax  8
#define error_start__system  9
/* TODO:[oc-merge] add error_start(foreign) in optim-comp */
#define error_start__foreign 10
#define error_start__user    11

/* TYPE_ERRORS */
#define TYPE_ERRORS(KEY) TYPE_ERRORS__##KEY
#define TYPE_ERRORS__atom 0
#define TYPE_ERRORS__atomic 1
#define TYPE_ERRORS__byte 2
#define TYPE_ERRORS__character 3
#define TYPE_ERRORS__compound 4
#define TYPE_ERRORS__evaluable 5
#define TYPE_ERRORS__in_byte 6
#define TYPE_ERRORS__integer 7
#define TYPE_ERRORS__list 8
#define TYPE_ERRORS__number 9
#define TYPE_ERRORS__predicate_indicator 10
#define TYPE_ERRORS__variable 11
#define TYPE_ERRORS__callable 12
//--
/* TODO:[oc-merge] unfold */
#define STRICT_ATOM TYPE_ERRORS(atom)
#define ATOMIC TYPE_ERRORS(atomic)
/* TODO:[oc-merge] renamed from BYTE, backport to optim-comp */
#define TY_BYTE TYPE_ERRORS(byte)
/* TODO:[oc-merge] used? */
#define CHARACTER TYPE_ERRORS(character)
#define COMPOUND TYPE_ERRORS(compound)
#define EVALUABLE TYPE_ERRORS(evaluable)
#define IN_BYTE TYPE_ERRORS(in_byte)
#define INTEGER TYPE_ERRORS(integer)
#define LIST TYPE_ERRORS(list)
#define NUMBER TYPE_ERRORS(number)
#define PREDICATE_INDICATOR TYPE_ERRORS(predicate_indicator)
/* RH: Not ISO anymore (from corrigendum 2) */
/* #define VARIABLE TYPE_ERRORS(variable) */
#define CALLABLE TYPE_ERRORS(callable)

/* DOMAIN_ERRORS */
#define DOMAIN_ERRORS(KEY) DOMAIN_ERRORS__##KEY
#define DOMAIN_ERRORS__character_code_list 0
#define DOMAIN_ERRORS__source_sink 1
#define DOMAIN_ERRORS__stream 2
#define DOMAIN_ERRORS__io_mode 3
#define DOMAIN_ERRORS__non_empty_list 4
#define DOMAIN_ERRORS__not_less_than_zero 5
#define DOMAIN_ERRORS__operator_priority 6
#define DOMAIN_ERRORS__prolog_flag 7
#define DOMAIN_ERRORS__read_option 8
#define DOMAIN_ERRORS__flag_value 9
#define DOMAIN_ERRORS__close_option 10
#define DOMAIN_ERRORS__stream_option 11
#define DOMAIN_ERRORS__stream_or_alias 12
#define DOMAIN_ERRORS__stream_position 13
#define DOMAIN_ERRORS__stream_property 14
#define DOMAIN_ERRORS__write_option 15
#define DOMAIN_ERRORS__operator_specifier 16
//--
/* TODO:[oc-merge] unfold */
/* TODO:[oc-merge] not in optim-comp */
#define CHARACTER_CODE_LIST     DOMAIN_ERRORS(character_code_list)
/* TODO:[oc-merge] not in optim-comp */
#define SOURCE_SINK             DOMAIN_ERRORS(source_sink)
/* TODO:[oc-merge] not in optim-comp */
#define STREAM                  DOMAIN_ERRORS(stream)
#define IO_MODE DOMAIN_ERRORS(io_mode)
/* TODO:[oc-merge] not renamed in optim-comp */
#define NON_EMPTY_LIST DOMAIN_ERRORS(non_empty_list)
#define NOT_LESS_THAN_ZERO DOMAIN_ERRORS(not_less_than_zero)
#define OPERATOR_PRIORITY DOMAIN_ERRORS(operator_priority)
#define PROLOG_FLAG DOMAIN_ERRORS(prolog_flag)
#define READ_OPTION DOMAIN_ERRORS(read_option)
#define FLAG_VALUE DOMAIN_ERRORS(flag_value)
#define CLOSE_OPTION DOMAIN_ERRORS(close_option)
#define STREAM_OPTION DOMAIN_ERRORS(stream_option)
#define STREAM_OR_ALIAS DOMAIN_ERRORS(stream_or_alias)
#define STREAM_POSITION DOMAIN_ERRORS(stream_position)
#define STREAM_PROPERTY DOMAIN_ERRORS(stream_property)
#define WRITE_OPTION DOMAIN_ERRORS(write_option)
#define OPERATOR_SPECIFIER DOMAIN_ERRORS(operator_specifier)

/* EXISTENCE_ERRORS */
#define EXISTENCE_ERRORS(KEY) EXISTENCE_ERRORS__##KEY
#define EXISTENCE_ERRORS__procedure 0
#define EXISTENCE_ERRORS__source_sink 1
#define EXISTENCE_ERRORS__stream 2
//--
#define PROCEDURE EXISTENCE_ERRORS(procedure)
/* TODO:[oc-merge] unfold */
/* SOURCE_SINK and STREAM already defined */
/* #define SOURCE_SINK EXISTENCE_ERRORS(source_sink) */
/* #define STREAM EXISTENCE_ERRORS(stream) */

/* PERMISION_ERRORS: composed of type of action + object on which the action
   is defined */

/* PERMISSION_TYPE */
#define PERMISSION_TYPES(KEY) PERMISSION_TYPES__##KEY
#define PERMISSION_TYPES__access 0
#define PERMISSION_TYPES__create 1
#define PERMISSION_TYPES__input 2
#define PERMISSION_TYPES__modify 3
#define PERMISSION_TYPES__open 4
#define PERMISSION_TYPES__output 5
#define PERMISSION_TYPES__reposition 6
//--
/* TODO:[oc-merge] unfold */
#define ACCESS PERMISSION_TYPES(access)
#define CREATE PERMISSION_TYPES(create)
#define INPUT PERMISSION_TYPES(input)
#define MODIFY PERMISSION_TYPES(modify)
#define OPEN PERMISSION_TYPES(open)
#define OUTPUT PERMISSION_TYPES(output)
#define REPOSITION PERMISSION_TYPES(reposition)

/* OBJECTS */
#define PERMISSION_OBJECTS(KEY) PERMISSION_OBJECTS__##KEY
#define PERMISSION_OBJECTS__binary_stream 0
#define PERMISSION_OBJECTS__source_sink 1
#define PERMISSION_OBJECTS__stream 2
#define PERMISSION_OBJECTS__text_stream 3
#define PERMISSION_OBJECTS__flag 4
#define PERMISSION_OBJECTS__operator 5
#define PERMISSION_OBJECTS__past_end_of_stream 6
#define PERMISSION_OBJECTS__private_procedure 7
#define PERMISSION_OBJECTS__static_procedure 8
//--
/* TODO:[oc-merge] unfold */
#define BINARY_STREAM PERMISSION_OBJECTS(binary_stream)
/* SOURCE_SINK and STREAM already defined */
/* #define SOURCE_SINK PERMISSION_OBJECTS(binary_stream) */
/* #define STREAM PERMISSION_OBJECTS(binary_stream) */
#define TEXT_STREAM PERMISSION_OBJECTS(text_stream)
#define FLAG PERMISSION_OBJECTS(flag)
#define OPERATOR PERMISSION_OBJECTS(operator)
#define PAST_END_OF_STREAM PERMISSION_OBJECTS(past_end_of_stream)
#define PRIVATE_PROCEDURE PERMISSION_OBJECTS(private_procedure)
#define STATIC_PROCEDURE PERMISSION_OBJECTS(static_procedure)

/* REPRESENTATION_ERROR */

/* CHARACTER_CODE_LIST already defined */
#define REPRESENTATION_ERRORS(KEY) REPRESENTATION_ERRORS__##KEY
#define REPRESENTATION_ERRORS__character_code_list 0
#define REPRESENTATION_ERRORS__in_character_code 1
#define REPRESENTATION_ERRORS__max_arity 2
#define REPRESENTATION_ERRORS__character 3
#define REPRESENTATION_ERRORS__max_integer 4
#define REPRESENTATION_ERRORS__min_integer 5
#define REPRESENTATION_ERRORS__character_code 6
#define REPRESENTATION_ERRORS__nan_or_inf_to_integer 7
#define REPRESENTATION_ERRORS__max_atom_length 8
//--
/* TODO:[oc-merge] unfold */
/* #define CHARACTER_CODE_LIST REPRESENTATION_ERRORS(character_code_list) */
#define IN_CHARACTER_CODE REPRESENTATION_ERRORS(in_character_code)
#define MAX_ARITY REPRESENTATION_ERRORS(max_arity)
/* #define CHARACTER REPRESENTATION_ERRORS(character) */
#define MAX_INTEGER REPRESENTATION_ERRORS(max_integer)
#define MIN_INTEGER REPRESENTATION_ERRORS(min_integer)
#define CHARACTER_CODE REPRESENTATION_ERRORS(character_code)
/* TODO:[oc-merge] new */
#define NAN_OR_INF_TO_INTEGER REPRESENTATION_ERRORS(nan_or_inf_to_integer)
#define MAX_ATOM_LENGTH       REPRESENTATION_ERRORS(max_atom_length)  /* Unneeded with dynamic atom sizes */

/* EVALUATION_ERROR */
#define EVALUATION_ERRORS(KEY) EVALUATION_ERRORS__##KEY
#define EVALUATION_ERRORS__float_overflow 0
#define EVALUATION_ERRORS__int_overflow 1
#define EVALUATION_ERRORS__e_undefined 2
#define EVALUATION_ERRORS__e_underflow 3
#define EVALUATION_ERRORS__zero_divisor 4
//--
/* TODO:[oc-merge] unfold */
#define FLOAT_OVERFLOW EVALUATION_ERRORS(float_overflow)
#define INT_OVERFLOW EVALUATION_ERRORS(int_overflow)
#define E_UNDEFINED EVALUATION_ERRORS(e_undefined)
#define E_UNDERFLOW EVALUATION_ERRORS(e_underflow)
#define ZERO_DIVISOR EVALUATION_ERRORS(zero_divisor)

/* TODO:[oc-merge] new */
/* RESOURCE_ERROR */
#define RESOURCE_ERRORS(KEY) RESOURCE_ERRORS__##KEY
#define RESOURCE_ERRORS__r_undefined 0
#define RESOURCE_ERRORS__r_stack 1
//--
/* TODO:[oc-merge] unfold */
#define R_UNDEFINED RESOURCE_ERRORS(r_undefined)
#define R_STACK RESOURCE_ERRORS(r_stack)

/* =========================================================================== */

/* OGRAMA: OLD VERSION ---------------------------------------------------- */ 
/* #define TYPE_ERROR(Type) (32+Type) */ /* includes also domain errors */ 

/* #define INSTANTIATION_ERROR 1
#define READ_PAST_EOS_ERROR 2
#define NO_READ_PERMISSION 3
#define NO_WRITE_PERMISSION 4
#define NO_SUCH_FILE 5
#define NO_OPEN_PERMISSION 6
#define NO_ACCESS_PERMISSION 7
#define SYSTEM_ERROR 8 OGRAMA */

/* Type codes for TYPE_ERROR  //) */
/* #define STRICT_ATOM 0
#define ATOMIC 1
#define TY_BYTE 2
#define CALLABLE 3
#define COMPOUND 4
#define EVALUABLE 5
#define IN_BYTE 6
#define INTEGER 7
#define LIST 8
#define NUMBER 9
#define PREDICATE_INDICATOR 10
#define VARIABLE 11 
*/

/*
#define CHARACTER_CODE_LIST 32 First domain code 
#define STREAM_OR_ALIAS 33
#define SOURCE_SINK 34  OGRAMA */
/* END OLD VERSION ----------------------------------------------------------- */

/* Exceptions (backport from optim_comp) */
/* TODO: pass worker as argument to macros? */

/* usage: goto, continue, break is forbidden inside CODE! */
#define EXCEPTION__CATCH(CODE, HANDLER) ({ \
  SIGJMP_BUF catch_exception__handler; \
  SIGJMP_BUF *catch_exception__old_handler; \
  catch_exception__old_handler = w->misc->errhandler; \
  w->misc->errhandler = &catch_exception__handler; \
  if (SIGSETJMP(catch_exception__handler)) { \
    /* just in case of a worker expansion */ \
    w = desc->worker_registers; \
    w->misc->errhandler = catch_exception__old_handler; \
    HANDLER; \
  } else { \
    CODE; \
  } \
})

#define UNLOCATED_EXCEPTION(Code) {             \
    ErrCode = Code;                             \
    ErrFuncName = "unknown";                    \
    ErrFuncArity = -1;                          \
    ErrArgNo = 0;                               \
    Culprit = TaggedZero;                       \
    EXCEPTION__THROW;                           \
  }

#define EXCEPTION__THROW SIGLONGJMP(*w->misc->errhandler, 1)

/* Throwing exceptions from builtins */

#define ERR__FUNCTOR(NAME, ARITY) \
  static char *const err__name = NAME; static const int err__arity = ARITY; 

#define BUILTIN_ERROR(Code,Culpr,ArgNo) ({ \
  ErrCode = Code; \
  ErrFuncName = err__name; \
  ErrFuncArity = err__arity; \
  ErrArgNo = ArgNo; Culprit = Culpr; \
  EXCEPTION__THROW; \
})

#define ERROR_IN_ARG(Arg,ArgNo,ReqType) ({ \
  BUILTIN_ERROR(IsVar(Arg) ? INSTANTIATION_ERROR : TYPE_ERROR(ReqType), \
                Arg, ArgNo); \
})

/* =========================================================================== */

/* MakeLST(To,Car,Cdr):
   
   Set 'To' to a term tagged_t LST
   whose car and cdr are 'Car' and Cdr'.

   'To' may be identical to 'Car' or 'Cdr'.
*/
#define MakeLST(To,Car,Cdr) \
{ tagged_t makelst_car = (Car); \
  HeapPush(w->heap_top,makelst_car); \
  HeapPush(w->heap_top,Cdr); \
  To = Tagp(LST,HeapOffset(w->heap_top,-2)); \
}

#define LSTCELLS 2 /* Cells allocated in MakeLST */

/* MakeSTR(To,Functor):
   
   Set 'To' to a term tagged_t STR
   whose principal functor is 'Functor'.  
   Space is allocated for the arguments, but they are not filled in.
*/
#define MakeSTR(To,Functor) \
{ \
  HeapPush(w->heap_top,Functor); \
  To = Tagp(STR,HeapOffset(w->heap_top,-1)); \
  w->heap_top = HeapOffset(w->heap_top,Arity(Functor)); \
}

#define CBOOL__UnifyCons(U,V) \
{ tagged_t m_t0, m_u=U, m_t1=V; \
  SwitchOnVar(m_t1,m_t0,{BindHVA(m_t1,m_u);}, \
                    {BindCVA(m_t1,m_u);}, \
                    {BindSVA(m_t1,m_u);}, \
                    {if (m_t1!=m_u) return FALSE;}) \
}

/* =========================================================================== */
/* Atom buffer - per worker temporary buffer to manipulate strings */

#define EXPAND_ATOM_BUFFER(new_max_atom_length) ({ \
  Atom_Buffer = checkrealloc_ARRAY(char, Atom_Buffer_Length, new_max_atom_length, Atom_Buffer); \
  Atom_Buffer_Length = new_max_atom_length; \
  UpdateHeapMargins(); \
})

/* Get a pointer to the atom buffer of at least LEN size, expand it
   before if required  */
#define GET_ATOM_BUFFER(S, LEN) ({ \
  if ((LEN) > Atom_Buffer_Length) { \
    EXPAND_ATOM_BUFFER((LEN)); \
  } \
  (S) = Atom_Buffer; \
})

/* Like GET_ATOM_BUFFER but extend Atom_Buffer a power of two */
#define GET_ATOM_BUFFER2(S, LEN) ({ \
  intmach_t alen=Atom_Buffer_Length; \
  while((LEN)>alen) { alen <<= 1; } \
  if (alen > Atom_Buffer_Length) { \
    EXPAND_ATOM_BUFFER(alen); \
  } \
  (S) = Atom_Buffer; \
})

/* Double atom buffer size if required, execute CODE if expanded */
/* Pre: LEN < Atom_Buffer_Length*2 */
#define ENSURE_ATOM_BUFFER(LEN, CODE) ({ \
  if ((LEN) >= Atom_Buffer_Length) { \
    EXPAND_ATOM_BUFFER(Atom_Buffer_Length*2); \
    CODE \
  } \
})

/* =========================================================================== */

#endif /* _CIAO_ENG_H */
