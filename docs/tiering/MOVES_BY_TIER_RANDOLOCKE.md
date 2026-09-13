# Moves by tier

Transcribed from `docs/tiering/community-moves-tierlist.png` (community-voted, 19 submitted
lists averaged, last updated 08/18/2026). **Edit this file, then regenerate** — never
hand-edit the generated C table.

```
python3 tools/randolocke/validate_tiers.py --moves        # check every name resolves
python3 tools/randolocke/tier_report.py --moves           # odds the weights produce
```

906 moves across the image's own six bands — 94% of this ROM's 959. Names are written as
they appear; the tools resolve them to `MOVE_*` constants and fail loudly on a mismatch.

**Known errors in the source image**, corrected by the validator's alias table:
`X-Scizzor` -> X-Scissor. A second `Curse???` tile duplicating Curse was dropped.

## Weights

```
python3 tools/randolocke/tier_report.py --moves
python3 tools/randolocke/tier_report.py --moves --weights="Niche=50,Bad=8"
```

Pool of 844 rollable moves; uniform draw is 0.1185% each.

| Tier | Count | Weight | Per move | vs uniform | Share of pool |
| --- | --- | --- | --- | --- | --- |
| Meta Defining | 4 | 1.18% | 0.2950% | **2.49x** | 0.5% |
| Staples | 46 | 10.90% | 0.2370% | **2.00x** | 5.5% |
| Filler/Outclassed | 203 | 42.09% | 0.2073% | **1.75x** | 24.1% |
| Niche | 383 | 40.15% | 0.1048% | 0.88x | 45.4% |
| Bad | 186 | 5.29% | 0.0284% | 0.24x | 22.0% |
| Pokemon Homeless | 22 | 0.39% | 0.0177% | 0.15x | 2.6% |

Niche lands at 0.88x rather than at uniform because the count-weighted average multiplier
must equal 1.0: raising the top three necessarily pulls the 383-move Niche band below
uniform. Any three of those four numbers can be held, not all four.

Meta Defining holds only 4 moves, so its weight is very sensitive. Tune it in steps of 0.1.

Re-run `tier_report.py --moves` after **any** worksheet change: the per-move rate is
weight/count, so adding moves to a tier silently dilutes it.

## "Pokemon Homeless" is the list's leftovers bin, not its worst tier

The band's own contents gave it away: it held Thunder Punch, Trick, Spirit Shackle, Roar of
Time and High Jump Kick, which are all perfectly good moves. On a list aggregated from 19
submissions, entries few people ranked sink to the bottom by default, so the bottom band is
closer to "unranked" than to "terrible".

Those five were moved up to **Bad**. Spacial Rend and Strength Sap were **excluded**
outright. What remains in the band is moves that genuinely do little, plus the nuzlocke
pushdowns.


## Exclusions applied

| Rule | Count | Why |
| --- | --- | --- |
| Z / Max / G-Max moves | 87 | Everything at or after `FIRST_Z_MOVE` in `include/constants/moves.h`. Phase 6 disabled Gigantamax and Randolocke lists Z-moves as unavailable, so these are inert. Detected structurally, not by name list. |
| Struggle | 1 | The game's fallback move, not a real option. |

## Nuzlocke pushdowns

Moved to Pokemon Homeless regardless of community placement, because a move that KOs its own
user costs a permanently dead Pokemon rather than a turn:

- **Self-KO:** Explosion, Self-Destruct, Memento, Misty Explosion, Final Gambit,
  Healing Wish, Lunar Dance
- **OHKO:** Fissure, Guillotine, Horn Drill, Sheer Cold

The community list is not nuzlocke-aware — it had Explosion and Final Gambit in *Niche*.
These live in `MOVES_PUSHDOWN` in `tools/randolocke/validate_tiers.py`, so they are applied
at generation time rather than by hand-editing this worksheet.

**Still to decide** (see RANDOLOCKE_PLAN.md 10.5 and 10.6):

All 846 rollable moves are now tiered; nothing is unreachable.

The 28 the community image does not cover are Gen 9 DLC (Teal Mask / Indigo Disk) plus
Misty Terrain, placed by hand:

- **Staples** — Ivy Cudgel, Blood Moon, Thunderclap, Electro Shot: high-power or priority
  signature moves that work on any species.
- **Filler/Outclassed** — the five Team Star Torque moves (100 BP with a status rider) plus
  Matcha Gotcha, Psychic Noise, Malignant Chain, Supercell Slam, Hard Press, Burning
  Bulwark, Psyblade, Mighty Cleave and Tera Starstorm. Tera Starstorm sits here rather than
  higher because `P_TERA_FORMS` is `FALSE`, so it is a plain 120 BP move.
- **Niche** — conditional or two-step moves: Upper Hand, Temper Flare, Alluring Voice,
  Hydro Steam, Tachyon Cutter, Fickle Beam, Syrup Bomb, Misty Terrain.
- **Bad** — Dragon Cheer, which only helps allies in a double battle.
- **Pokemon Homeless** — Hold Hands, which does nothing.

## Meta Defining
Boomburst, Extreme Speed, Belly Drum, Protect

## Staples
Aqua Jet, Glare, Facade, Body Slam, Fake Out, Bulk Up, Perish Song, Aurora Veil,
Close Combat, Earthquake, Future Sight, Crunch,
Astonish, Agility, Hyper Voice, Growth, Bounce, Aeroblast, Bug Buzz, Dazzling Gleam,
Ancient Power, Rage Fist, Bullet Punch, Eruption,
Giga Drain, Shadow Ball, Recover, Baneful Bunker, Astral Barrage, Endure, Giga Impact,
Dragon Dance, Acid Spray, Follow Me, Foul Play, First Impression,
Leech Seed, Swords Dance, Attack Order, Trick Room, Aromatherapy, Drain Punch,
Ivy Cudgel, Blood Moon, Thunderclap, Electro Shot

## Filler/Outclassed
Baton Pass, Hyper Beam, Brave Bird, Helping Hand, Aerial Ace, Air Slash, Baby-Doll Eyes,
Rapid Spin, Slack Off, Yawn, Population Bomb, Acrobatics,
Bolt Strike, Earth Power, Moonblast, Hex, Amnesia, Disarming Voice, Shadow Sneak,
Scary Face, Nasty Plot, Court Change, Ceaseless Edge, Draco Meteor,
Belch, Chatter, Curse, Recycle, Detect, Bitter Malice, Flash Cannon, Blizzard,
Discharge, Disable, Knock Off, Accelerock,
Clear Smog, Flamethrower, Arm Thrust, Ominous Wind, Freeze-Dry, Toxic Spikes, Anchor Shot,
Bite, Blast Burn, Bone Rush, Gravity, Muddy Water,
Substitute, Flip Turn, Fire Blast, High Horsepower, Bulldoze, Aqua Tail, Cross Poison,
Grass Knot, Flare Blitz, Brick Break, Blaze Kick, Shadow Bone,
Sludge Wave, Thunderbolt, Rest, Outrage, Wish, Work Up, Scald, Lunge, Fire Lash,
Brutal Swing, Howl, Will-O-Wisp,
Doom Desire, Heavy Slam, Blue Flare, Guillotine, Air Cutter, Fly, Toxic, Return,
Poison Jab, Gigaton Hammer, Stealth Rock, Last Respects,
Dire Claw, Expanding Force, Brine, Dark Void, Aqua Ring, Confide, Dive, Dragon Claw,
Morning Sun, Tri Attack, Pain Split, Nuzzle,
Shed Tail, Dragon Ascent, Heat Wave, Diamond Storm, Freezing Glare, Circle Throw,
Hydro Cannon, Shadow Force, Spore, Raging Fury, Shell Smash, Sticky Web,
Tidy Up, Make It Rain, Ally Switch, Clanging Scales, Spikes, Ice Shard, Mach Punch,
Fiery Wrath, Dragon Tail, Acid Armor, Shadow Claw, Super Fang,
Tickle, Night Daze, Sacred Sword, Apple Acid, Electroweb, Darkest Lariat, Geomancy,
Hurricane, After You, Breaking Swipe, Bullet Seed, Endeavor,
Flail, Horn Leech, Dig, Quick Attack, Soft-Boiled, Zen Headbutt, Behemoth Blade, Defog,
Leaf Blade, Energy Ball, Frustration, Beat Up,
Eerie Spell, Mirror Coat, Charge Beam, Dizzy Punch, Moonlight, Shadow Punch, Thunder Wave,
Sucker Punch, Quiver Dance, Light That Burns The Sky, Ice Beam, Leaf Storm,
Copycat, Heal Bell, Extrasensory, Flame Charge, Light Screen, Beak Blast, Fissure,
Counter, Gastro Acid, Corrosive Gas, Electro Ball, Lock-On
,
10000000 Volt Thunderbolt, Slash, Thrash, Stomp, Raging Bull, Rollout, Water Spout,
Taunt, Weather Ball, Overheat, Hyper Drill, Double Iron Bash,
Core Enforcer, Horn Drill, Cosmic Power, Dynamic Punch, Frenzy Plant, Drill Run,
Dragon Pulse, Growl, Head Smash, Strength, Torment,
Revenge, Psychic Terrain, Salt Cure,
Matcha Gotcha, Psychic Noise, Malignant Chain, Supercell Slam,
Hard Press, Burning Bulwark, Psyblade, Mighty Cleave, Tera Starstorm,
Blazing Torque, Combat Torque, Magical Torque, Noxious Torque, Wicked Torque

## Niche
Behemoth Bash, False Surrender, Moongeist Beam, Burn Up, Entrainment, Jaw Lock, Meteor Mash,
Reflect, Thief, Power-Up Punch, Mystical Fire, Glacial Lance,
Assurance, Grassy Glide, Cotton Spore, Barrier, Block, Defend Order, Explosion, Fire Spin,
Focus Energy, Fury Cutter, Bone Club, Double-Edge,
Dragon Breath, Fire Fang, Iron Head, Teleport, Pursuit, X-Scizzor, Superpower, Punishment,
Payback, Tailwind, Parting Shot, Mortal Spin,
Hydro Pump, Avalanche, Absorb, Dragon Rush, Egg Bomb, False Swipe, Grassy Terrain, Headbutt,
Icicle Crash, Nightmare, Power Gem, Slam,
Poison Gas, Sleep Talk, Poison Fang, Wood Hammer, Relic Song, Glaciate, Barb Barrage,
Cotton Guard, Fusion Flare, Fiery Dance, Fake Tears, Bleakwind Storm,
Feint, Bug Bite, Captivate, Crabhammer, Leech Life, Lick, Mud Shot, Phantom Force, Spite,
Whirlwind, Sunny Day, Confusion,
Sweet Kiss, Snarl, Refresh, Retaliate, Psyshock, Bolt Beak, Coil, Calm Mind, Fusion Bolt,
Liquidation, Flatter, Lash Out,
Mean Look, Acid, Cross Chop, Harden, Infestation, Screech, Sleep Powder, Psystrike,
Safeguard, Toxic Thread, Swagger, U-turn
,
Psycho Cut, Victory Dance, Throat Chop, Esper Wing, Head Charge, Instruct, Infernal Parade,
Confuse Ray, Crush Grip, Minimize, Drill Peck, Ember,
Ice Fang, Ice Hammer, Sing, Surf, Thunder, Thunderous Kick, Sandstorm, Rain Dance,
Smack Down, All-Out Pummeling, Axe Kick, Ruination,
Clangorous Soulblaze, King's Shield, Aura Wheel, Clangorous Soul, Drum Beating, Icy Wind,
Feather Dance, Clamp, Assist, Destiny Bond, Bide, Chloroblast,
Dragon Hammer, Flame Wheel, Hammer Arm, Magical Leaf, Mind Reader, Roar, Smart Strike,
Night Slash, Stored Power, Stone Edge, Light of Ruin, Steam Eruption,
Acid Downpour, Trop Kick, Let's Snuggle Forever, Psychic Fangs, Spirit Break, Jet Punch,
Fling, Lumina Crash, Fleur Cannon, Decorate, Final Gambit, Metal Sound,
Autotomize, Imprison, Aromatic Mist, Bubble Beam, Flying Press, Ice Ball, Lovely Kiss,
Poltergeist, Corkscrew Crash, Twister, Spark, Roost,
Volt Switch, Soak, Bloom Doom, Aura Sphere, Body Press, Gyro Ball, Heat Crash,
Hyperspace Fury, Acupressure, Grudge, Hyper Fang, Ingrain,
Camouflage, Chip Away, Crush Claw, Electric Terrain, Iron Defense, Land's Wrath, Leer,
Stun Spore, Take Down, Rock Slide, Vacuum Wave, Genesis Supernova,
Steel Wing, Sacred Fire, Sheer Cold, Psycho Boost, Secret Sword, Stone Axe, Dynamax Cannon,
Fell Stinger, Meteor Beam, Covet, Echoed Voice, Flash,
Foresight, Freeze Shock, Solar Beam, Triple Kick, Signal Beam, Spiky Shield, Pollen Puff,
Tearful Look, Fillet Away, Baddy Bad, Floral Healing, Heal Order,
Dragon Darts, Extreme Evoboost, Bestow, Icicle Spear, Flame Burst, Force Palm, Lucky Chant,
Metal Claw, Mud-Slap, Spectral Thief, Trick-or-Treat, Never-Ending Nightmare
,
Soul-Stealing 7-Star Strike, Strange Steam, Thunder Fang, Rock Smash, Teeter Dance,
Waterfall, Stomping Tantrum, Steel Beam, Trailblaze, Dark Pulse, Dragon Energy, Focus Punch,
Forest's Curse, Me First, Meteor Assault, Bubble, Conversion 2, Iron Tail, Tackle,
String Shot, Rock Tomb, Precipice Blades, Psycho Shift, Oblivion Wing,
Tar Shot, No Retreat, Fishious Rend, Dream Eater, Fire Pledge, Aurora Beam, Horn Attack,
Low Sweep, Mega Kick, Steel Roller, Swift, Sharpen,
Sky Uppercut, Smelling Salts, Sand Tomb, Rock Blast, Mystical Power, Simple Beam, Round,
Rock Polish, Pound, Parabolic Charge, Sparkling Aria, Shell Trap,
Venoshock, Triple Axel, Aqua Step, Bonemerang, Bouncy Bubble, Celebrate, Comet Punch,
Grass Pledge, Guard Split, Flower Shield, Charge, Inferno,
Leafage, Menacing Moonraze Maelstrom, Water Gun, Volt Tackle, Poison Powder, Sweet Scent,
Whirlpool, Origin Pulse, Wring Out, Take Heart, Razor Shell, Secret Power,
Rage Powder, Snap Trap, Play Nice, Thousand Arrows, Wave Crash, Pounce, Aqua Cutter,
Chilling Water, Glitzy Glow, Multi-Attack, Burning Jealousy, Eternabeam,
Magic Coat, Magic Room, Barrage, Lava Plume, Magma Storm, Mega Drain, Mist Ball,
Night Shade, Scratch, Pay Day, Switcheroo, Self-Destruct,
Synthesis, Role Play, Tail Glow, Luster Purge, Struggle Bug, Thousand Waves,
Breakneck Blitz, Revelation Dance, Thunder Cage, Double Shock, Ice Spinner, Grav Apple,
Crafty Shield, Encore, Hyperspace Hole, Fury Swipes, Guard Swap, Last Resort, Haze,
Life Dew, Mist, Seismic Toss, Psybeam, Vital Throw,
Razor Leaf, Vise Grip, Chilly Reception, Surging Strikes, Silk Trap, Twin Beam,
Collision Course, Buzzy Buzz, Eerie Impulse, Triple Arrows, Frost Breath, Heal Block
,
Conversion, Feint Attack, Fairy Wind, Gust, Mountain Gale, Petal Dance, Tectonic Rage,
Rock Throw, Spider Web, Psych Up, Telekinesis, Plasma Fists,
Scale Shot, Wicked Blow, Heal Pulse, Coaching, Happy Hour, Magic Powder, Ice Burn,
Sinister Arrow Raid, Poison Sting, Wrap, Shell Side Arm, Rage,
Reversal, Snore, V-create, Water Shuriken, Prismatic Laser, Stuff Cheeks, Scorching Sands,
Terrain Pulse, Flower Trick,
Upper Hand, Temper Flare, Alluring Voice, Hydro Steam, Tachyon Cutter,
Fickle Beam, Syrup Bomb, Misty Terrain

## Bad
Freezy Frost, Low Kick, Memento, Hone Claws, Magnet Rise, Cut, Subzero Slammer, Zap Cannon,
Powder Snow, Uproar, Trump Card, Rock Climb,
Tail Slap, Noble Roar, Twinkle Tackle, Sunsteel Strike, Photon Geyser, Rising Voltage,
Doodle, Order Up, Kowtow Cleave, Gunk Shot, Mat Block, Embargo,
Fury Attack, Metal Burst, Branch Poke, Double Hit, Dual Chop, Electrify, Floaty Fall,
Karate Chop, Mud Sport, Water Pledge, Peck, Sludge,
Transform, Octazooka, Odor Sleuth, Shock Wave, Poison Tail, Snatch, Rock Wrecker,
Natural Gift, Wild Charge, Shift Gear, Techno Blast, Gigavolt Havoc,
Petal Blizzard, Comeuppance, G-Max Resonance, Headlong Rush, Heart Swap, Ion Deluge,
Mind Blown, Incinerate, Leaf Tornado, Mega Punch, Stoked Sparksurfer, Sparkly Swirl,
Thunder Shock, Silver Wind, Nature Power, Wake-Up Slap, Hail, Wildbolt Storm, Powder,
Solar Blade, Snowscape, Torch Song, Armor Cannon, Bitter Blade,
Tera Blast, Electro Drift, Constrict, Metronome, Mimic, Mirror Move, Defense Curl,
Dragon Rage, Gear Grind, Grass Whistle, Meditate, Miracle Eye,
Skull Bash, Withdraw, Rolling Kick, Needle Arm, Reflect Type, Quash, Searing Shot,
Storm Throw, Pluck, Shore Up, Pyro Ball, Octolock
,
Spin Out, Magnet Bomb, Magnetic Flux, Mirror Shot, Misty Explosion, G-Max Terror,
Splishy Splash, Sappy Seed, Sizzly Slide, Pin Missile, Sketch, Skill Swap,
Sandsear Storm, Water Pulse, Malicious Moonsault, Nature's Madness, Overdrive, Venom Drench,
Revival Blessing, Fire Punch, Magnitude, Tail Whip, Wing Attack, Zippy Zap,
Sonic Boom, Splash, Present, Snipe Shot, Triple Dive, Focus Blast, Milk Drink, Charm,
Lunar Blessing, Dual Wingbeat, Wide Guard, Splintered Stormshards,
Hydro Vortex, Inferno Overdrive, Devastating Drake, Searing Sunraze Smash, Double Slap,
Gear Up, Laser Focus, Spike Cannon, Worry Seed, Synchronoise, Sky Drop, Topsy-Turvy,
Shelter, Savage Spin-Out, Zing Zap, Power Trip, Glaive Rush, Megahorn, Healing Wish,
Jump Kick, Lunar Dance, Pika Papow, Sand Attack, Veevee Volley,
Water Sport, Vine Whip, Wonder Room, Oceanic Operetta, Speed Swap, Obstruct, Heart Stamp,
Jungle Healing, Twineedle, Power Swap, Psyshield Bash, Springtide Storm,
Hidden Power, Hold Back, Judgment, Stockpile, Swallow, Rototiller, Supersonic Skystrike,
Spotlight, Purify, Guardian of Alola, Skitter Smack, Teatime,
Bind, Kinesis, G-Max Befuddle, Supersonic, Sky Attack, Spit Up, Quick Guard, Power Split,
Draining Kiss, G-Max Vine Lash, G-Max Wildfire, G-Max Cannonade,
G-Max Volt Crash, G-Max Gold Rush, G-Max Chi Strike, Max Mindstorm, Max Starfall,
G-Max Foam Burst, G-Max Cuddle, G-Max Replenish, Spicy Extract, Seed Bomb, Seed Flare,
Max Airstream,
Attract, Max Phantasm, Razor Wind, Catastropika, Psywave, Sludge Bomb, Hypnosis, Psychic,
Struggle, Play Rough, Max Geyser, Ice Punch
,
Power Whip, G-Max Malodor,
Dragon Cheer,
Thunder Punch, Trick, Spirit Shackle, Roar of Time, High Jump Kick

## Pokemon Homeless
Double Team, Pulverizing Pancake, Power Shift, G-Max Meltdown, Continental Crush, Max Darkness, Double Kick, Submission, Shattered Psyche, G-Max Drum Solo, G-Max Fireball, G-Max Hydrosnipe, Max Guard, G-Max Wind Rage, G-Max Gravitas, Max Overgrowth, Max Strike, Max Rockfall, G-Max Stonesurge, Max Lightning, G-Max Volcalith, G-Max Tartness, G-Max Sweetness, G-Max Sandblast, Max Ooze, G-Max Stun Shock, Max Flare, G-Max Centiferno, G-Max Smite, G-Max Snooze, Max Steelspike, Max Knuckle, Max Quake, G-Max Finale, Max Flutterby, Max Hailstorm, G-Max Steelsurge, G-Max Depletion, Max Wyrmwind, G-Max One Blow, G-Max Rapid Flow, Black Hole Eclipse, Mud Bomb, Steamroller, Fairy Lock, Smokescreen, Smog, Power Trick, Hold Hands
