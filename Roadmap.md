# Roadmap

At time of writing this I acknowledge that progress on Clara has been slow-to-non-existent since the initial re-write -
and I apologise for that (especially since I've publicised it a fair bit).

To make the way forward clearer I'm going to present a roadmap here for features that I definitely plan to get to in
the relatively near-term - although I can't promise deadlines!

These mostly coincide with current outstanding issues in the tracker (which I'll link), but not including bug-fixes
(which should also be addressed, of course).

Currently this doesn't necessarily reflect the order I attempt to work through them in (although I expect the milestone
structure to be reasonably stable) - but I'll start trying to organise them into some sort of order.

I've been talking in the first person here because, at this stage, I'm not quite ready to throw development fully
open to the community effort (much as I'm grateful to all offers for help, and existing PRs). That's not to say
that I won't be accepting PRs - I'll be working through them to see which I can accept as-is, which might need
modification, and which will need to be approached differently.
This is because I still have a vision of where I'm aiming to go - and how to get there. Part of this process will
be communicating that - but in the near-term it's easier if I do the driving to get to at least the first milestone -
and maybe the second. This may include some heavy refactoring of some of the internals.

Like all good roadmaps, it's not really a map at all, but a route - and I've taken placenames starting from the town I
live in, going in a certain direction.

### Milestone 1: Tonbridge
 
- Add more documentation (includes [#49](https://github.com/catchorg/Clara/issues/49))
- Finish work on "required" parsers [#39](https://github.com/catchorg/Clara/issues/39), [#16](https://github.com/catchorg/Clara/issues/16) & [PR #28](https://github.com/catchorg/Clara/pull/28)
- Finish work on removing exceptions completely (or making them "optional")
- (compiler conditional) Support for `std::optional` and maybe other optional types (e.g. boost) [#4](https://github.com/catchorg/Clara/issues/4) & [PR #45](https://github.com/catchorg/Clara/pull/45)
- Arg\[0] support on Windows [#29](https://github.com/catchorg/Clara/issues/29)
 
### Milestone 2: Maidstone

- config files and environment variables
- "hidden" options [#29](https://github.com/catchorg/Clara/issues/29)
- Capture general text for help output [#48]((https://github.com/catchorg/Clara/issues/48)

### Milestone 3: Ashford

- subcommands [#31](https://github.com/catchorg/Clara/issues/31) (I actually have a design mostly done for this) - but see also [PR #32](https://github.com/catchorg/Clara/pull/32)

---

