# Abilities by tier

Transcribed from `docs/tiering/community-abilities-tierlist.png` (community-voted list).
**Edit this file, then regenerate** — never hand-edit `src/data/randomizer/ability_tiers.h`.

```
python3 tools/randolocke/validate_tiers.py      # check every name resolves
python3 tools/randolocke/gen_ability_tiers.py   # write the C table
```

Names are written as they appear on the image. The tools resolve them to `ABILITY_*`
constants and report anything that does not match, so a misreading fails loudly.

**Known errors in the source image**, corrected by the alias table in the validator:
`Aerialate` -> Aerilate, `WonderSkin` -> Wonder Skin, `Magma Veil` -> Magma Armor.

**Excluded regardless of tier:** Wonder Guard (a randomized Shedinja ability trivialises or
bricks fights) — carried over from the pokeemerald_rando_enh tables.

**The Negative tier is never rolled.** It carries weight 0, so those abilities stay listed
and auditable here but cannot be selected. Give it a non-zero weight to re-enable it.

**Randolocke-specific placements**, where config or format matters more than competitive
strength:
- `P_TERA_FORMS` is `FALSE`, so Tera Shift and all four Embody Aspect masks cannot trigger.
- Commander, Costar and Hospitality are doubles-only, and this is a singles run.
- Supreme Overlord scales with fainted teammates, which a nuzlocke is played to avoid, and
  Anger Shell drops defences at half HP — both pushed below their competitive placement.
- Sharpness and other move-dependent boosts are worth less when movesets are randomized.

## Weights

```
python3 tools/randolocke/tier_report.py                      # odds these weights produce
python3 tools/randolocke/tier_report.py --weights=S=12,A=20  # try different ones
```

Current: **S 9 / A 20 / B 32 / C 24 / D 11 / F 4 / Negative 0**, over a 308-ability pool.

| Tier | Count | Weight | Per ability | vs uniform |
| --- | --- | --- | --- | --- |
| S | 12 | 9% | 0.7500% | **2.31x** |
| A | 31 | 20% | 0.6452% | 1.99x |
| B | 78 | 32% | 0.4103% | 1.26x |
| C | 87 | 24% | 0.2759% | 0.85x |
| D | 64 | 11% | 0.1719% | 0.53x |
| F | 36 | 4% | 0.1111% | 0.34x |
| Negative | 7 | 0 | never | — |

S is deliberately held close to A (1.16x apart, not the 1.4x an earlier draft had): the point
is that good abilities are *more likely*, not that S becomes the expected outcome.

A tier's weight is **split across its members**, so a large tier dilutes itself. The first
weights tried here (S 8 / A 22) made an A ability *more* likely than an S one — S had 12
members against A's 31. `tier_report.py` fails if a lower tier out-draws a higher one, so
that class of mistake cannot ship silently.

## S
Drizzle, Huge Power, Drought, Intimidate, Levitate, Arena Trap, Adaptability, Speed Boost,
Orichalcum Pulse, Hadron Engine, Good as Gold, Eelevate

## A
Guts, Wonder Guard, Pure Power, Shadow Tag, Sand Stream, Magic Bounce, Compound Eyes,
Serene Grace, Prankster, Magic Guard, Desolate Land, Contrary,
Download, Beast Boost, Clear Body, Gorilla Tactics, Flash Fire, Poison Heal, Chlorophyll,
Disguise, Delta Stream, Defiant, Regenerator, Libero,
Protosynthesis, Sword of Ruin, Beads of Ruin, Tablets of Ruin, Vessel of Ruin,
Tera Shell, Purifying Salt, Earth Eater

## B
Moxie, Natural Cure, No Guard, Fur Coat, Technician, Protean, Primordial Sea, Lightning Rod,
Aerialate, Effect Spore, Water Absorb, Magnet Pull,
Swift Swim, Mold Breaker, Flame Body, Imposter, Intrepid Sword, Sturdy, Sheer Force,
Volt Absorb, Battle Armor, Competitive, Dry Skin, Parental Bond,
Thick Fat, Multiscale, Rough Skin, Ice Scales, Electric Surge, Unaware, Marvel Scale,
Iron Barbs, Moody, Pixilate, Filter, Simple,
Galvanize, Snow Warning, Grassy Surge, Tinted Lens, Grim Neigh, Fluffy, Skill Link, As One,
Psychic Surge, Illusion, Dauntless Shield, Scrappy,
Trace, Dragon's Maw, Misty Surge, Blaze, Hustle, Chilling Neigh, Static, Refrigerate,
Immunity, Dark Aura, Rock Head, Unburden,
Inner Focus, Water Bubble, Cursed Body, Iron Fist, Fairy Aura, Harvest, Storm Drain,
Tough Claws,
Quark Drive, Sharpness, Well-Baked Body, Thermal Exchange, Toxic Chain, Rocky Payload,
As One (Shadow Rider), Dragonize, Fire Mane, Mega Sol

## C
Infiltrator, Motor Drive, Comatose, Heatproof, Insomnia, Synchronize, Cloud Nine,
Neutralizing Gas, Gale Wings, Corrosion, Sand Rush, Mirror Armor,
Overgrow, Mega Launcher, Triage, Air Lock, Berserk, Limber, Analytic, Torrent, Bulletproof,
Shield Dust, Pressure, Battle Bond,
Bad Dreams, Sap Sipper, Soundproof, Cheek Pouch, Hydration, Sand Veil, Solid Rock,
Soul-Heart, Reckless, Super Luck, Hyper Cutter, Strong Jaw,
Shed Skin, Full Metal Body, White Smoke, Poison Point, Neuroforce, Steelworker, Shell Armor,
Gluttony, Dazzling, Stamina, Shadow Shield, Rain Dish,
Solar Power, Sniper, Early Bird, Punk Rock, Frisk, Oblivious, Liquid Ooze, Transistor,
Flare Boost, Cute Charm, Damp, Poison Touch,
Unseen Fist, Queenly Majesty, Sand Force, Swarm, Aftermath, Own Tempo, Quick Feet, Teravolt,
Gooey, Vital Spirit, Liquid Voice, Turboblaze,
Justified, Steely Spirit, Innards Out, Slush Rush, Toxic Boost,
Wind Rider, Electromorphosis, Toxic Debris, Armor Tail, Guard Dog, Mind's Eye,
Opportunist, Seed Sower, Piercing Drill, Spicy Spray

## D
Water Veil, Overcoat, Prism Armor, Quick Draw, Ice Body, Anger Point, Keen Eye, Victory Star,
Stakeout, Dancer, Long Reach, Snow Cloak,
Surge Surfer, Gulp Missile, Sticky Hold, Mummy, Aroma Veil, Merciless, Stance Change,
Cotton Down, Multitype, Friend Guard, Leaf Guard, Pickpocket,
Suction Cups, WonderSkin, Unnerve, Weak Armor, Magma Veil, Color Change, Flower Gift, Stench,
Perish Body, Steam Engine, Magician, Sand Spit,
Pastel Veil, Power Construct, Tangling Hair, Wandering Spirit, Ripen, Steadfast, Forewarn,
Screen Cleaner, Rattled, Forecast, Anticipation, Water Compaction,
Ice Face, Heavy Metal, Sweet Veil, Big Pecks, Grassy Pelt, Normalize,
Wind Power, Anger Shell, Cud Chew, Lingering Aroma, Mycelium Might, Poison Puppeteer,
Supersweet Syrup, Teraform Zero, Supreme Overlord, Zero to Hero

## F
Light Metal, Aura Break, Telepathy, Pickup, Healer, Rivalry, RKS System, Battery,
Tangled Feet, Flower Veil, Curious Medicine, Power Spot,
Minus, Plus, Emergency Exit, Propeller Tail, Hunger Switch, Power of Alchemy, Shields Down,
Mimicry, Receiver, Stalwart, Run Away, Symbiosis,
Zen Mode, Schooling, Illuminate, Klutz,
Commander, Costar, Hospitality, Tera Shift, Embody Aspect Teal Mask,
Embody Aspect Wellspring Mask, Embody Aspect Hearthflame Mask,
Embody Aspect Cornerstone Mask

## Negative
Wimp Out, Ball Fetch, Honey Gather, Truant, Stall, Defeatist, Slow Start
