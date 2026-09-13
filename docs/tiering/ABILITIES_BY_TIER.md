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

## S
Drizzle, Huge Power, Drought, Intimidate, Levitate, Arena Trap, Adaptability, Speed Boost

## A
Guts, Wonder Guard, Pure Power, Shadow Tag, Sand Stream, Magic Bounce, Compound Eyes,
Serene Grace, Prankster, Magic Guard, Desolate Land, Contrary,
Download, Beast Boost, Clear Body, Gorilla Tactics, Flash Fire, Poison Heal, Chlorophyll,
Disguise, Delta Stream, Defiant, Regenerator, Libero

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
Tough Claws

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
Justified, Steely Spirit, Innards Out, Slush Rush, Toxic Boost

## D
Water Veil, Overcoat, Prism Armor, Quick Draw, Ice Body, Anger Point, Keen Eye, Victory Star,
Stakeout, Dancer, Long Reach, Snow Cloak,
Surge Surfer, Gulp Missile, Sticky Hold, Mummy, Aroma Veil, Merciless, Stance Change,
Cotton Down, Multitype, Friend Guard, Leaf Guard, Pickpocket,
Suction Cups, WonderSkin, Unnerve, Weak Armor, Magma Veil, Color Change, Flower Gift, Stench,
Perish Body, Steam Engine, Magician, Sand Spit,
Pastel Veil, Power Construct, Tangling Hair, Wandering Spirit, Ripen, Steadfast, Forewarn,
Screen Cleaner, Rattled, Forecast, Anticipation, Water Compaction,
Ice Face, Heavy Metal, Sweet Veil, Big Pecks, Grassy Pelt, Normalize

## F
Light Metal, Aura Break, Telepathy, Pickup, Healer, Rivalry, RKS System, Battery,
Tangled Feet, Flower Veil, Curious Medicine, Power Spot,
Minus, Plus, Emergency Exit, Propeller Tail, Hunger Switch, Power of Alchemy, Shields Down,
Mimicry, Receiver, Stalwart, Run Away, Symbiosis,
Zen Mode, Schooling, Illuminate, Klutz

## Negative
Wimp Out, Ball Fetch, Honey Gather, Truant, Stall, Defeatist, Slow Start
