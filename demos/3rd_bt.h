// Behavior trees: decision planning and decision making.
// Supersedes finite state-machines (FSM) and hierarchical finite state-machines (HFSM).
// - rlyeh, public domain.
//
// [ref] https://outforafight.wordpress.com/2014/07/15/behaviour-behavior-trees-for-ai-dudes-part-1/
// [ref] https://www.gameaipro.com/GameAIPro/GameAIPro_Chapter06_The_Behavior_Tree_Starter_Kit.pdf
// [ref] https://gitlab.com/NotYetGames/DlgSystem/-/wikis/Dialogue-Manual
// [ref] https://towardsdatascience.com/designing-ai-agents-behaviors-with-behavior-trees-b28aa1c3cf8a
// [ref] https://docs.nvidia.com/isaac/packages/behavior_tree/doc/behavior_trees.html
// [ref] https://docs.unrealengine.com/4.26/en-US/InteractiveExperiences/ArtificialIntelligence/BehaviorTrees/
// [ref] gdc ChampandardDaweHernandezCerpa_BehaviorTrees.pdf @todo debugging
// [ref] https://docs.cocos.com/cocos2d-x/manual/en/actions/

#if 0

The nodes in a behavior tree can be broadly categorized into three main types: control nodes, task nodes, and decorator nodes. Here is a brief description of each category:

Control Nodes: Control nodes are used to control the flow of the behavior tree. They determine the order in which child nodes are executed and how the results of those nodes are combined. They are usually parent nodes.
Action Nodes: Action nodes are used to perform specific actions or tasks within the behavior tree. They can include actions such as moving, attacking, or interacting with objects in the game world. They are usually leaf nodes.
Decorator Nodes: Decorator nodes are used to modify the behavior of child nodes in some way. They can be used to repeat child nodes, invert the result of a child node, or add a cooldown period between executions. They are usually located between control and action nodes.

--- VARIABLES

Key Prefixes:
    {visiblity:(l)ocal,s(q)uad,(r)ace,(f)action,(g)lobal}
    [persistence:(t)emp,(u)serdata,(s)avegame,(c)loud] + '_' + name.
    [persistence:(tmp),(usr)data,(sav)egame,(net)cloud] + '_' + name.

Ie, l_health = 123.4, gsav_player = "john"

--- ACTION NODES

[ ] * Actions/Behaviors have a common structure:
[ ]     - Entry point (function name) for a C call or Lua script.
[ ] * Status:
[ ]     - Uninitialized (never run)
[ ]     - Running (in progress)
[ ]     - Suspended (on hold till resumed)
[ ]     - Success (finished and succeeded)
[ ]     - Failure (finished and failed)
[ ] * Optional callbacks:
[ ]     - on_enter
[ ]     - on_leave
[ ]     - on_success
[ ]     - on_failure
[ ]     - on_suspend
[ ]     - on_resume

[x] Action Node: This node performs a single action, such as moving to a specific location or attacking a target.
[ ] Blackboard Node: Node that reads and writes data to a shared memory space known as a blackboard. The blackboard can be used to store information that is relevant to multiple nodes in the behavior tree.
[ ]     SetKey(keyVar,object)
[ ]     HasKey(keyVar)
[ ]     CompareKeys(keyVar1, operator < <= > >= == !=, keyVar2)
[ ]     SetTags(names=blank,cooldownTime=inf,bIsCooldownAdditive=false)
[ ]     HasTags(names=blank,bAllRequired=true)
[ ]     PushToStack(keyVar,itemObj): creates a new stack if one doesn’t exist, and stores it in the passed variable name, and then pushes ‘item’ object onto it.
[ ]     PopFromStack(keyVar,itemVar): pop pops an item off the stack, and stores it in the itemVar variable, failing if the stack is already empty.
[ ]     IsEmptyStack(keyVar): checks if the stack passed is empty and returns success if it is, and failure if its not.
[ ] Communication Node: This is a type of action node that allows an AI agent to communicate with other agents or entities in the game world. The node takes an input specifying the message to be communicated and the recipient(s) of the message (wildmask,l/p/f/g prefixes). The node then sends the message to the designated recipient(s) and returns success when the communication is completed. This node can be useful for implementing behaviors that require the AI agent to coordinate with other agents or to convey information to the player. It could use a radius argument to specify the maximum allowed distance for the recipients.
[ ] Condition Node: A leaf node that checks a specific condition, such as the distance to an object, the presence of an enemy, or the status of a health bar.
[ ]     Distance Condition Node: This is a type of condition node that evaluates whether an AI agent is within a specified distance of a target object or location. The node takes two inputs: the current position of the AI agent and the position of the target object or location. If the distance between the two is within a specified range, the node returns success. If the distance is outside of the specified range, the node returns failure. This node can be useful for implementing behaviors that require the AI agent to maintain a certain distance from a target, such as following or avoiding an object. Could use a flag to disambiguate between linear distance and path distance.
[ ]     Query Node: This node checks a condition and returns success or failure based on the result. For example, a query node could check whether an enemy is within range or whether a door is locked.
[ ]         Query Node: A type of decorator node that retrieves information from a database or external system, such as a web service or file system. The Query node can be used to retrieve data that is not available within the game engine, such as weather or traffic conditions.
[ ]     A condition is made of:
[ ]     - Optional [!] negate
[ ]     - Mandatory Value1(Int/Flt/Bool/VarName/FuncName)
[ ]     - Optional operator [< <= > >= == !=] and Value2(Int/Flt/Bool/VarName/FuncName)
[ ]     AllConditions(...) : SUCCESS if ( empty array || all conditions met)
[ ]     AnyConditions(...) : SUCCESS if (!empty array && one conditions met)

--- DECORATOR NODES

[ ] Cooldown Node: Decorator node that adds a cooldown period between the execution of a child node, preventing it from being executed again until the cooldown period has elapsed.
[x] Counter Node: Decorator node that limits the number of times that a child node can execute. For example, if the child node has executed a certain number of times, the Counter node will return a failure.
[x]     Once Node: Decorator node that triggers a specified action when a condition is met for the first time. This can be useful for triggering a one-time event, such as when an AI agent discovers a hidden item or reaches a new area of the game world.
[x] Inverter Node: Decorator node that inverts the result of a child node, returning success if the child node fails and failure if the child node succeeds. This can be useful for negating the outcome of a particular behavior, such as avoiding a certain area of the game world.
[x] Repeater Node: Decorator node that repeats the execution of a child node a specified number of times or indefinitely.
[x]     Repeat(times=inf): Runs child node given times. These are often used at the very base of the tree, to make the tree to run continuously.
[ ]         RepeatIf(strong/weak condition): Runs child node as long as the conditions are met.
[ ]         RepeatIfOk(times=inf): Runs child node if it succeedes, max given times.
[ ]         RepeatIfFail(times=inf): Runs child node if it fails, max given times.
[ ] Branch Node: 2 children [0] for true, [1] for false
[ ]     Resource Node: Decorator node that manages a shared resource, such as a limited supply of ammunition or a limited amount of processing power. The Resource node The node can be used to decide when to use or conserve the resource. For example, if the AI agent is low on ammunition, the node may decide to switch to a melee weapon or to retreat to a safer location. This node can be useful for implementing behaviors that require the AI agent to make strategic decisions about resource use.
[x] Result Node: Decorator node that tracks the result of a child action node and returns either success or failure depending on the outcome. This can be useful for determining whether an action was successful or not, and then adjusting the AI agent's behavior accordingly.
[x]     Succeeder Node: Decorator node that always returns success, regardless of the result of its child node. This can be useful for ensuring that certain behaviors are always executed, regardless of the overall success or failure of the behavior tree.
[x]     Success(): FAILURE becomes SUCCESS (TRUE).
[x]     Failure(): SUCCESS becomes FAILURE (FALSE).
[ ] Throttle Node: Decorator node that limits the rate at which a child node can execute. For example, a Throttle node might ensure that an AI agent can only fire its weapon, or using a special ability, only a certain number of times per second.
[x] Delay Node: Decorator node that adds a delay to the execution of a child node. The delay might be configured to sleep before the execution, after the execution, or both.
[x]     Defer Delay(duration_ss=1): Runs the child node, then sleeps for given seconds.
[ ] Ease(time,name): Clamps child time to [0,1] range, and applies easing function on it.
[ ] Dilate(Mul=1,Add=0): Dilates child time

--- CONTROL NODES

[x] Root Node: The topmost node in a behavior tree that represents the start of the decision-making process. Returns success if any of its child nodes suceedes.
[x] Root-Failure Node: Control node that only returns failure if all of its child nodes fail. This can be useful for ensuring that a behavior tree is not prematurely terminated if one or more child nodes fail.

[ ] Event(name): When name event is raised, it suspends current tree and calls child. "Incoming projectile" -> Evade. Stimulus types: may be disabled by event, or autodisabled.
[ ] Raise(name): Triggers event name.

[ ] Checkpoint Node: Control node that saves a state in memory and then continues the tree execution from that point the next time the tree is executed. It can be useful in situations where the behavior tree needs to be interrupted and resumed later.
[ ] Decision Making Node: Control node that implements a decision-making process for the AI agent. The node takes input specifying the available options for the AI agent and the criteria for evaluating each option. The node then evaluates each option based on the specified criteria and selects the best option. This node can be useful for implementing behaviors that require the AI agent to make strategic decisions, such as choosing a target or selecting a path through the game world.
[ ]     Could be extended with GOAP if dynamically inserting the scores on each update then calling a Probability Selector Node (0.2,0.3,0.5)
[ ]     https://cdn.cloudflare.steamstatic.com/apps/valve/2012/GDC2012_Ruskin_Elan_DynamicDialog.pdf
[ ] Evaluate Node / Recheck():
[ ]     Actively rechecks all existing sub-conditions on a regular basis after having made decisions about them.
[ ]     Use this feature to dynamically check for risks or opportunities in selected parts of the tree.
[ ]     For example, interrupting a patrol with a search behavior if a disturbance is reported.
[ ] Interrupt Node: Control node that interrupts the execution of a lower-priority node when a higher-priority node needs to be executed. It can be useful for handling urgent tasks or emergency situations.
[ ] Monitor Node: Control node that continuously monitors a condition and triggers a specified action when the condition is met. For example, a Monitor node might monitor the distance between an AI agent and a target and trigger a "retreat" behavior when the distance falls below a certain threshold.
[ ]     Observer Node: Control node that monitors the state of the game world and triggers a specified action when certain conditions are met. For example, an Observer node might trigger a "hide" behavior when an enemy is spotted.
[ ] Parallel Node: Control node that executes multiple child nodes simultaneously. The Parallel node continues to execute even if some of its child nodes fail.
[ ]     Parallel All Node(required_successes=100%): Control node that executes multiple child nodes simultaneously. Returns false when first child fails, and aborts any other running tasks. Returns true if all its children succeed.
[ ]     Parallel One Node(required_successes=1): Control node that executes multiple child nodes simultaneously. Returns true when first child suceedes, and aborts any other running tasks. Returns false if all its children fail.
[ ]     Subsumption Node: Control node that allows multiple behaviors to be executed simultaneously, with higher-priority behaviors taking precedence over lower-priority ones. This can be useful for implementing complex, multi-level behaviors in autonomous systems.
[ ] Semaphore Node: Control node that blocks the execution of its child nodes until a certain condition is met. For example, a Semaphore node might block the execution of a behavior until a certain object is in range or a certain event occurs.
[ ] Semaphore Wait Node: Control node that blocks the execution of its child nodes until a resource becomes available. This can be useful for controlling access to shared resources such as a pathfinding system or a communication channel.
[ ]     WaitTags(tags=blank,timeout_ss=inf): Stops execution of child node until the cooldown tag(s) do expire. May return earlier if timed out.
[ ]     WaitEvent(name=blank,timeout_ss=inf): Stops execution of child node until event is raised. May return earlier if timed out.
[x] Sequence Node(reversed,iterator(From,To)): Control node that executes a series of child nodes in order. If any of the child nodes fail, the Sequence node stops executing and returns a failure.
[ ]     Dynamic Sequence Node: Control node that dynamically changes the order in which its child nodes are executed based on certain conditions. For example, a Dynamic Sequence node might give higher priority to a child node that has recently failed or that is more likely to succeed.
[ ]     Reverse(): iterates children in reversed order (Iterator:(-1,0) equivalent)
[ ]     Iterator(from,to): allows negative indexing. increments if (abs from < abs to), decrements otherwise
[x] Selector Node: Control node that selects a child node to execute based on a predefined priority or set of conditions. The Selector node stops executing child nodes as soon as one of them succeeds.
[ ]     Priority Selector Node: Control node that executes its child nodes in order of priority. If the first child node fails, it moves on to the next child node in the list until a successful node is found. This can be useful for implementing behaviors that require a specific order of execution, such as a patrol route or a search pattern.
[ ]     Probability Selector Node: Control node that selects a child node to execute based on a probability distribution. For example, if there are three child nodes with probabilities of 0.2, 0.3, and 0.5, the Probability Selector node will execute the third child node 50% of the time.
[ ]     Dynamic Selector Node: Control node that dynamically changes the order in which its child nodes are executed based on certain conditions. For example, a Dynamic Selector node might give higher priority to a child node that has recently succeeded or that is more likely to succeed.
[ ]         Dynamic Selector Node: Control node that dynamically changes the order in which child nodes are executed based on certain conditions. For example, it may prioritize certain nodes when health is low, and other nodes when resources are scarce.
[ ]         Dynamic Priority Node: Control node that dynamically changes the priority of its child nodes based on certain conditions. For example, if a child node is more likely to result in success, the Dynamic Priority node will give it a higher priority.
[ ]     Weighted / Cover Selection Node: This is a type of selector node that evaluates the available cover options in the game world and selects the best cover position for the AI agent. The node takes input specifying the current position of the AI agent and the location of potential cover positions. The node then evaluates the cover positions based on criteria such as distance to enemy positions, line of sight, and available cover points, and selects the best option. This node can be useful for implementing behaviors that require the AI agent to take cover and avoid enemy fire.
[ ]     Random Selector Node: Control node that selects a child node to execute at random. This can be useful for introducing variety and unpredictability into the behavior of an AI agent.
[ ]     Random Weight / Stochastic Node(0.2,0.3,0.5): Control node that introduces randomness into the decision-making process of an AI agent. For example, a Stochastic node might randomly select a child node to execute, with the probability of each child node being proportional to its likelihood of success.
[ ] Break(bool): breaks parent sequence or selector. may return SUCCESS or FAILURE.
[ ] Hybrid Node: Control node that combines the functionality of multiple control nodes, such as a sequence node and a selector node. It can be useful for creating complex behavior patterns that require multiple control structures. The Hybrid Node has two modes of operation: strict mode and flexible mode. In strict mode, the child nodes are executed in a fixed order, similar to a Sequence Node. If any child node fails, the entire Hybrid Node fails. In flexible mode, the child nodes can be executed in any order, similar to a Selector Node. If any child node succeeds, the entire Hybrid Node succeeds. The Hybrid Node is often used when you need to create complex behavior patterns that require multiple control structures. For example, you might use a Hybrid Node to implement a search behavior where the AI agent first moves in a fixed direction for a certain distance (strict mode), but then switches to a more exploratory mode where it randomly explores nearby areas (flexible mode).
[ ] Subtree Node: Control node that calls another behavior tree as a subroutine. This is useful for breaking down complex behaviors into smaller, more manageable parts.
[ ]     Call(name): Calls to the child node with the matching name within behavior tree. Returns FAILURE if name is invalid or if invoked behavior returns FAILURE.
[ ]     Return(boolean): Exits current Call.
[ ]     AttachTree(Name,bDetachAfterUse=true): Attaches subtree to main tree. When a NPC founds the actor, it attaches the behavior to the tree. Ie, Level specific content: Patrols, Initial setups, Story driven events, etc. Ie, DLCs: Behaviors are added to actors in the DLC level (enticers).
[ ]     DetachTree(Name)
[ ] Switch Node: Control node that evaluates a condition and then selects one of several possible child nodes to execute based on the result of the condition.
[ ]     Switch(name): Jumps to the child node with the matching name within behavior tree. If name is invalid will defer to Fallback child node.
[ ] Timeout Node: Control node that aborts the execution of its child node after a certain amount of time has elapsed. This can be useful for preventing an AI agent from getting stuck in a loop or waiting indefinitely for a particular event to occur.
[ ]     TimeLimit(timeout_ss=inf): Give the child any amount of time to finish before it gets canceled. The timer is reset every time the node gains focus.
[ ] Timer Node: Control node which invokes its child node every XX seconds. The timer can repeat the action any given number of times, or indefinitely.

## Proposal -----------------------------------------------------------------------------

BehaviorTrees as case-insensitive INI files. Then,
- INI -> C INTERPRETER, OR
- INI -> LUA TRANSPILER -> LUA BYTECODE -> LUA VM.

```ini
[bt]
recheck
sequence                                  ;; Approach the player if seen!
 conditions=IsPlayerVisible,
 action=MoveTowardsPlayer
sequence                                  ;; Attack the player if seen!
 conditions=IsPlayerInRange,
 repeat=3
 action=FireAtPlayer
sequence                                  ;; Search near last known position
 conditions=HaveWeGotASuspectedLocation,
 action=MoveToPlayersLastKnownPosition
 action=LookAround
sequence                                  ;; Randomly scanning nearby
 action=MoveToRandomPosition
 action=LookAround
event=IncomingProjectile
 action=Evade
```

#endif

typedef int (*bt_func)();
map(char*, bt_func) binds;

void bt_addfun(const char *name, int(*func)()){
    do_once map_init_str(binds);
    map_find_or_add_allocated_key(binds, STRDUP(name), func);
}
bt_func bt_findfun(const char *name) {
    bt_func *found = map_find(binds, (char*)name);
    return found ? *found : 0;
}
char * bt_funcname(bt_func fn) {
    for each_map(binds,char*,k,bt_func,f) {
        if( f == fn ) return k;
    }
    return 0;
}

typedef struct bt_t {
    uint64_t type;
    int (*action)();
    union {
        int argi;
        float argf;
    };
    //struct bt_t *parent;
    array(struct bt_t) children;
} bt_t;

bt_t bt(const char *ini_file, unsigned flags) {
    bt_t z = {0}, root = z;
    array(char*) m = strsplit(vfs_read(ini_file), "\r\n");

    bt_t *self = &root;
    self->type = cc8(root);
    //self->parent = self;

    for( int i = 0; i < array_count(m); ++i ) {
        // parse ini
        int level = strspn(m[i], " \t");
        char *k = m[i] + level;
        if( k[0] == '[' ) {
            if( strcmp(k+1, "bt]") ) return z; // we only support [bt]
            continue;
        }
        int sep = strcspn(k, " =:");
        char *v = k + sep; if(sep) *v++ = '\0'; else v = k + strlen(k); // v = (char*)"";
        array(char*) args = *v ? strsplit(v, " ") : NULL;

        // insert node in tree
        bt_t *out = self;
        while( level-- ) {
            out = array_back(out->children);
        }
        array_push(out->children, ((bt_t){0}));
        //array_back(out->children)->parent = out;
        out = array_back(out->children);

        // config node
        out->type = *(uint64_t*)va("%-8s", k);
        if( array_count(args) ) out->argf = atof(args[0]);
        if( !strcmp(k, "run") ) out->action = bt_findfun(v);
    }

    return root;
}

int bt_run(bt_t *b) {
    int rc = 0;

    /**/ if( b->type == cc8(     run) ) { return b->action ? b->action() : 0; }
    else if( b->type == cc8(     not) ) { return !bt_run(b->children + 0); }
    else if( b->type == cc8(   sleep) ) { return sleep_ss(b->argf), bt_run(b->children + 0); }
    else if( b->type == cc8(   defer) ) { rc = bt_run(b->children + 0); return sleep_ss(b->argf), rc; }
    else if( b->type == cc8(    loop) ) { int rc; for(int i = 0; i < b->argf; ++i) rc = bt_run(b->children + 0); return rc; }
    else if( b->type == cc8(    once) ) { return b->argf ? 0 : (b->argf = 1), bt_run(b->children + 0); }
    else if( b->type == cc8(   count) ) { return b->argf <= 0 ? 0 : --b->argf, bt_run(b->children + 0); }
    else if( b->type == cc8(    pass) ) { return bt_run(b->children + 0), 1; }
    else if( b->type == cc8(    fail) ) { return bt_run(b->children + 0), 0; }
    else if( b->type == cc8(  result) ) { return bt_run(b->children + 0), !!b->argf; }
    else if( b->type == cc8(     all) ) { for( int i = 0; i < array_count(b->children); ++i ) if(!bt_run(b->children+i)) return 0; return 1; }
    else if( b->type == cc8(     any) ) { for( int i = 0; i < array_count(b->children); ++i ) if( bt_run(b->children+i)) return 1; return 0; }
    else if( b->type == cc8(    root) ) { for( int i = 0; i < array_count(b->children); ++i ) rc|=bt_run(b->children+i); return rc; }
    else if( b->type == cc8(rootfail) ) { rc = 1; for( int i = 0; i < array_count(b->children); ++i ) rc&=~bt_run(b->children+i); return rc; }

    return 0;
}

void ui_bt(bt_t *b) {
    if( b ) {
        char *info = bt_funcname(b->action);
        if(!info) info = va("%d", array_count(b->children));

        if( ui_collapse(va("%s (%s)", cc8str(b->type), info), va("bt%p",b)) ) {
            for( int i = 0; i < array_count(b->children); ++i) {
                ui_bt(b->children + i);
            }
            ui_collapse_end();
        }
    }
}
