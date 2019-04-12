#define Ray_Main TraceClosestHit
#define Ray_Shadow TraceAnyHit

#if RAYTRACE_HAS_CLOSEST_HIT

Payload_s TraceClosestHit(vec3 rO, vec3 rV, int Flags = 0, float MaxLength = 100000)
{
	int Depth = 0;

	Context_s Context;
	Context.Payload = InitPayload();
	Context.Ray.Origin = rO;
	Context.Ray.Direction = rV;

#if RAYTRACE_HAS_FACE_CULLING
	Context.FaceCullingValue = Flags == 0 ? 0.f : ((Flags & RAY_BACKFACE_CULL) != 0 ? 1.0 : -1.0);
#endif

	do
	{
		Context.Ray.Direction = normalize(Context.Ray.Direction);
		Context.Result = InitRayResult(MaxLength);
		Context.bValidClosest = false;
		Context.bValidAny = false;

		IntersectSceneBvh(Context, RAY_CLOSEST_HIT);
		barrier();

		if (Context.bValidAny)
		{
			return Context.Payload;
		}

		if (Context.bValidClosest)
		{
			Context.Ray = ClosestHit(Context.Ray, Context.Result, Context.Material, RAY_MAIN, ++Depth, Context.Payload);
			if (dot(Context.Ray.Direction, Context.Ray.Direction) > 0.1)
				continue;
		}
        break;
	} while (true);

	barrier();

	if (Context.bValidClosest == false)
	{
		Miss(Context.Ray, RAY_MAIN, Context.Payload);
	}

	barrier();

	return Context.Payload;
}

#endif

Payload_s TraceAnyHit(vec3 rO, vec3 rV, int Flags = 0, float MaxLength = 100000)
{
	Context_s Context;
	Context.Payload = InitPayload();
	Context.Result = InitRayResult(MaxLength);
	Context.Ray.Origin = rO;
	Context.Ray.Direction = normalize(rV);

#if RAYTRACE_HAS_FACE_CULLING
	Context.FaceCullingValue = Flags == 0 ? 0.f : ((Flags & RAY_BACKFACE_CULL) != 0 ? 1.0 : -1.0);
#endif

	Context.bValidClosest = false;
	Context.bValidAny = false;

	IntersectSceneBvh(Context, RAY_ANY_HIT);

	if (!(Context.bValidAny || Context.bValidClosest))
	{
		Miss(Context.Ray, RAY_ANY_HIT, Context.Payload);
	}

	return Context.Payload;
}
