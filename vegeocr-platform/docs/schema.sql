
\restrict muZFe1jw8E0gPiPSSD5bemguUHjd5XlPB7NARA5clI1O5SAEeaTSVchGFwVEHrr

-- Name: api_keys; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.api_keys (
    id integer NOT NULL,
    user_id integer NOT NULL,
    name character varying(80) NOT NULL,
    prefix character varying(12) NOT NULL,
    key_hash character varying(64) NOT NULL,
    rate_limit_per_min integer NOT NULL,
    monthly_quota integer NOT NULL,
    created_at timestamp with time zone NOT NULL,
    last_used_at timestamp with time zone,
    revoked_at timestamp with time zone,
    scopes json,
    expires_at timestamp with time zone,
    enabled boolean DEFAULT true NOT NULL,
    last4 character varying(4),
    rotated_at timestamp with time zone
);

-- Name: api_keys_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.api_keys_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: api_keys_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.api_keys_id_seq OWNED BY public.api_keys.id;

-- Name: api_request_logs; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.api_request_logs (
    id integer NOT NULL,
    api_key_id integer,
    user_id integer,
    method character varying(8) NOT NULL,
    path character varying(200) NOT NULL,
    status_code integer NOT NULL,
    latency_ms integer NOT NULL,
    created_at timestamp with time zone NOT NULL
);

-- Name: api_request_logs_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.api_request_logs_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: api_request_logs_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.api_request_logs_id_seq OWNED BY public.api_request_logs.id;

-- Name: dataset_models; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.dataset_models (
    id integer NOT NULL,
    dataset_id integer NOT NULL,
    version integer NOT NULL,
    status character varying(10) NOT NULL,
    path character varying(500),
    cer_before double precision,
    cer_after double precision,
    train_samples integer NOT NULL,
    test_samples integer NOT NULL,
    iterations integer NOT NULL,
    active boolean NOT NULL,
    message text NOT NULL,
    created_at timestamp with time zone NOT NULL,
    finished_at timestamp with time zone
);

-- Name: dataset_models_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.dataset_models_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: dataset_models_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.dataset_models_id_seq OWNED BY public.dataset_models.id;

-- Name: dataset_samples; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.dataset_samples (
    id integer NOT NULL,
    dataset_id integer NOT NULL,
    image_path character varying(500) NOT NULL,
    text text NOT NULL,
    original_text text,
    source character varying(12) NOT NULL,
    created_at timestamp with time zone NOT NULL,
    updated_at timestamp with time zone NOT NULL
);

-- Name: dataset_samples_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.dataset_samples_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: dataset_samples_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.dataset_samples_id_seq OWNED BY public.dataset_samples.id;

-- Name: datasets; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.datasets (
    id integer NOT NULL,
    user_id integer NOT NULL,
    name character varying(80) NOT NULL,
    category character varying(40) NOT NULL,
    description text NOT NULL,
    lang character varying(20) NOT NULL,
    created_at timestamp with time zone NOT NULL,
    updated_at timestamp with time zone NOT NULL
);

-- Name: datasets_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.datasets_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: datasets_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.datasets_id_seq OWNED BY public.datasets.id;

-- Name: lab_runs; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.lab_runs (
    id character varying(32) NOT NULL,
    user_id integer NOT NULL,
    filename character varying(255) NOT NULL,
    mime character varying(60) NOT NULL,
    mode character varying(12) NOT NULL,
    lang character varying(40) NOT NULL,
    enhance character varying(6) NOT NULL,
    engine character varying(12) NOT NULL,
    dataset_id integer,
    status character varying(12) NOT NULL,
    error_code character varying(40),
    error text,
    result json,
    created_at timestamp with time zone NOT NULL,
    finished_at timestamp with time zone
);

-- Name: learning_modules; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.learning_modules (
    id integer NOT NULL,
    slug character varying(80) NOT NULL,
    title character varying(160) NOT NULL,
    topic character varying(60) NOT NULL,
    difficulty character varying(14) NOT NULL,
    summary text NOT NULL,
    "position" integer NOT NULL,
    published boolean NOT NULL,
    created_at timestamp with time zone NOT NULL
);

-- Name: learning_modules_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.learning_modules_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: learning_modules_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.learning_modules_id_seq OWNED BY public.learning_modules.id;

-- Name: lesson_progress; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.lesson_progress (
    id integer NOT NULL,
    user_id integer NOT NULL,
    lesson_id integer NOT NULL,
    last_viewed_at timestamp with time zone,
    completed_at timestamp with time zone
);

-- Name: lesson_progress_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.lesson_progress_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: lesson_progress_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.lesson_progress_id_seq OWNED BY public.lesson_progress.id;

-- Name: lessons; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.lessons (
    id integer NOT NULL,
    module_id integer NOT NULL,
    "position" integer NOT NULL,
    title character varying(160) NOT NULL,
    content text NOT NULL,
    minutes integer NOT NULL
);

-- Name: lessons_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.lessons_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: lessons_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.lessons_id_seq OWNED BY public.lessons.id;

-- Name: model_versions; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.model_versions (
    id integer NOT NULL,
    lang character varying(20) NOT NULL,
    version integer NOT NULL,
    path character varying(500) NOT NULL,
    status character varying(12) NOT NULL,
    trigger character varying(12) NOT NULL,
    cer double precision,
    baseline_cer double precision,
    train_samples integer NOT NULL,
    test_samples integer NOT NULL,
    notes text NOT NULL,
    created_at timestamp with time zone NOT NULL,
    activated_at timestamp with time zone
);

-- Name: model_versions_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.model_versions_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: model_versions_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.model_versions_id_seq OWNED BY public.model_versions.id;

-- Name: ocr_jobs; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.ocr_jobs (
    id character varying(32) NOT NULL,
    user_id integer NOT NULL,
    api_key_id integer,
    source character varying(8) NOT NULL,
    filename character varying(255) NOT NULL,
    mime character varying(60) NOT NULL,
    file_path character varying(500) NOT NULL,
    lang character varying(40) NOT NULL,
    status character varying(12) NOT NULL,
    error text,
    page_count integer NOT NULL,
    text text NOT NULL,
    edited_text text,
    avg_confidence double precision,
    result json,
    created_at timestamp with time zone NOT NULL,
    finished_at timestamp with time zone
);

-- Name: password_resets; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.password_resets (
    id integer NOT NULL,
    user_id integer NOT NULL,
    token_hash character varying(64) NOT NULL,
    expires_at timestamp with time zone NOT NULL,
    used_at timestamp with time zone
);

-- Name: password_resets_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.password_resets_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: password_resets_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.password_resets_id_seq OWNED BY public.password_resets.id;

-- Name: quiz_attempts; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.quiz_attempts (
    id integer NOT NULL,
    user_id integer NOT NULL,
    quiz_id integer NOT NULL,
    api_key_id integer,
    correct integer NOT NULL,
    total integer NOT NULL,
    score integer NOT NULL,
    passed boolean NOT NULL,
    answers json NOT NULL,
    created_at timestamp with time zone NOT NULL
);

-- Name: quiz_attempts_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.quiz_attempts_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: quiz_attempts_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.quiz_attempts_id_seq OWNED BY public.quiz_attempts.id;

-- Name: quiz_questions; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.quiz_questions (
    id integer NOT NULL,
    quiz_id integer NOT NULL,
    "position" integer NOT NULL,
    prompt text NOT NULL,
    kind character varying(10) NOT NULL,
    options json NOT NULL,
    correct json NOT NULL,
    explanation text NOT NULL
);

-- Name: quiz_questions_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.quiz_questions_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: quiz_questions_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.quiz_questions_id_seq OWNED BY public.quiz_questions.id;

-- Name: quizzes; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.quizzes (
    id integer NOT NULL,
    lesson_id integer NOT NULL,
    title character varying(160) NOT NULL,
    pass_percent integer NOT NULL
);

-- Name: quizzes_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.quizzes_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: quizzes_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.quizzes_id_seq OWNED BY public.quizzes.id;

-- Name: settings; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.settings (
    key character varying(60) NOT NULL,
    value json NOT NULL
);

-- Name: training_runs; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.training_runs (
    id integer NOT NULL,
    lang character varying(20) NOT NULL,
    status character varying(10) NOT NULL,
    trigger character varying(12) NOT NULL,
    requested_by integer,
    started_at timestamp with time zone,
    finished_at timestamp with time zone,
    created_at timestamp with time zone NOT NULL,
    version_id integer,
    message text NOT NULL
);

-- Name: training_runs_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.training_runs_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: training_runs_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.training_runs_id_seq OWNED BY public.training_runs.id;

-- Name: training_samples; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.training_samples (
    id integer NOT NULL,
    user_id integer,
    api_key_id integer,
    job_id character varying(32),
    source character varying(12) NOT NULL,
    lang character varying(20) NOT NULL,
    image_path character varying(500) NOT NULL,
    text text NOT NULL,
    original_text text,
    status character varying(10) NOT NULL,
    is_test boolean NOT NULL,
    used_in_version_id integer,
    created_at timestamp with time zone NOT NULL,
    reviewed_at timestamp with time zone
);

-- Name: training_samples_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.training_samples_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: training_samples_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.training_samples_id_seq OWNED BY public.training_samples.id;

-- Name: users; Type: TABLE; Schema: public; Owner: -

CREATE TABLE public.users (
    id integer NOT NULL,
    email character varying(255) NOT NULL,
    password_hash character varying(100) NOT NULL,
    full_name character varying(120) NOT NULL,
    role character varying(10) NOT NULL,
    status character varying(10) NOT NULL,
    created_at timestamp with time zone NOT NULL,
    last_login_at timestamp with time zone,
    username character varying(40),
    token_version integer DEFAULT 0 NOT NULL,
    password_changed_at timestamp with time zone
);

-- Name: users_id_seq; Type: SEQUENCE; Schema: public; Owner: -

CREATE SEQUENCE public.users_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

-- Name: users_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -

ALTER SEQUENCE public.users_id_seq OWNED BY public.users.id;

-- Name: api_keys id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.api_keys ALTER COLUMN id SET DEFAULT nextval('public.api_keys_id_seq'::regclass);

-- Name: api_request_logs id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.api_request_logs ALTER COLUMN id SET DEFAULT nextval('public.api_request_logs_id_seq'::regclass);

-- Name: dataset_models id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.dataset_models ALTER COLUMN id SET DEFAULT nextval('public.dataset_models_id_seq'::regclass);

-- Name: dataset_samples id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.dataset_samples ALTER COLUMN id SET DEFAULT nextval('public.dataset_samples_id_seq'::regclass);

-- Name: datasets id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.datasets ALTER COLUMN id SET DEFAULT nextval('public.datasets_id_seq'::regclass);

-- Name: learning_modules id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.learning_modules ALTER COLUMN id SET DEFAULT nextval('public.learning_modules_id_seq'::regclass);

-- Name: lesson_progress id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.lesson_progress ALTER COLUMN id SET DEFAULT nextval('public.lesson_progress_id_seq'::regclass);

-- Name: lessons id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.lessons ALTER COLUMN id SET DEFAULT nextval('public.lessons_id_seq'::regclass);

-- Name: model_versions id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.model_versions ALTER COLUMN id SET DEFAULT nextval('public.model_versions_id_seq'::regclass);

-- Name: password_resets id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.password_resets ALTER COLUMN id SET DEFAULT nextval('public.password_resets_id_seq'::regclass);

-- Name: quiz_attempts id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.quiz_attempts ALTER COLUMN id SET DEFAULT nextval('public.quiz_attempts_id_seq'::regclass);

-- Name: quiz_questions id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.quiz_questions ALTER COLUMN id SET DEFAULT nextval('public.quiz_questions_id_seq'::regclass);

-- Name: quizzes id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.quizzes ALTER COLUMN id SET DEFAULT nextval('public.quizzes_id_seq'::regclass);

-- Name: training_runs id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.training_runs ALTER COLUMN id SET DEFAULT nextval('public.training_runs_id_seq'::regclass);

-- Name: training_samples id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.training_samples ALTER COLUMN id SET DEFAULT nextval('public.training_samples_id_seq'::regclass);

-- Name: users id; Type: DEFAULT; Schema: public; Owner: -

ALTER TABLE ONLY public.users ALTER COLUMN id SET DEFAULT nextval('public.users_id_seq'::regclass);

-- Name: api_keys api_keys_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.api_keys
    ADD CONSTRAINT api_keys_pkey PRIMARY KEY (id);

-- Name: api_request_logs api_request_logs_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.api_request_logs
    ADD CONSTRAINT api_request_logs_pkey PRIMARY KEY (id);

-- Name: dataset_models dataset_models_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.dataset_models
    ADD CONSTRAINT dataset_models_pkey PRIMARY KEY (id);

-- Name: dataset_samples dataset_samples_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.dataset_samples
    ADD CONSTRAINT dataset_samples_pkey PRIMARY KEY (id);

-- Name: datasets datasets_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.datasets
    ADD CONSTRAINT datasets_pkey PRIMARY KEY (id);

-- Name: lab_runs lab_runs_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.lab_runs
    ADD CONSTRAINT lab_runs_pkey PRIMARY KEY (id);

-- Name: learning_modules learning_modules_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.learning_modules
    ADD CONSTRAINT learning_modules_pkey PRIMARY KEY (id);

-- Name: learning_modules learning_modules_slug_key; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.learning_modules
    ADD CONSTRAINT learning_modules_slug_key UNIQUE (slug);

-- Name: lesson_progress lesson_progress_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.lesson_progress
    ADD CONSTRAINT lesson_progress_pkey PRIMARY KEY (id);

-- Name: lessons lessons_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.lessons
    ADD CONSTRAINT lessons_pkey PRIMARY KEY (id);

-- Name: model_versions model_versions_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.model_versions
    ADD CONSTRAINT model_versions_pkey PRIMARY KEY (id);

-- Name: ocr_jobs ocr_jobs_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.ocr_jobs
    ADD CONSTRAINT ocr_jobs_pkey PRIMARY KEY (id);

-- Name: password_resets password_resets_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.password_resets
    ADD CONSTRAINT password_resets_pkey PRIMARY KEY (id);

-- Name: quiz_attempts quiz_attempts_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.quiz_attempts
    ADD CONSTRAINT quiz_attempts_pkey PRIMARY KEY (id);

-- Name: quiz_questions quiz_questions_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.quiz_questions
    ADD CONSTRAINT quiz_questions_pkey PRIMARY KEY (id);

-- Name: quizzes quizzes_lesson_id_key; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.quizzes
    ADD CONSTRAINT quizzes_lesson_id_key UNIQUE (lesson_id);

-- Name: quizzes quizzes_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.quizzes
    ADD CONSTRAINT quizzes_pkey PRIMARY KEY (id);

-- Name: settings settings_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.settings
    ADD CONSTRAINT settings_pkey PRIMARY KEY (key);

-- Name: training_runs training_runs_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.training_runs
    ADD CONSTRAINT training_runs_pkey PRIMARY KEY (id);

-- Name: training_samples training_samples_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.training_samples
    ADD CONSTRAINT training_samples_pkey PRIMARY KEY (id);

-- Name: users users_pkey; Type: CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (id);

-- Name: ix_api_keys_key_hash; Type: INDEX; Schema: public; Owner: -

CREATE UNIQUE INDEX ix_api_keys_key_hash ON public.api_keys USING btree (key_hash);

-- Name: ix_api_keys_user_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_api_keys_user_id ON public.api_keys USING btree (user_id);

-- Name: ix_api_request_logs_api_key_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_api_request_logs_api_key_id ON public.api_request_logs USING btree (api_key_id);

-- Name: ix_api_request_logs_created_at; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_api_request_logs_created_at ON public.api_request_logs USING btree (created_at);

-- Name: ix_api_request_logs_user_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_api_request_logs_user_id ON public.api_request_logs USING btree (user_id);

-- Name: ix_dataset_models_dataset_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_dataset_models_dataset_id ON public.dataset_models USING btree (dataset_id);

-- Name: ix_dataset_samples_dataset_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_dataset_samples_dataset_id ON public.dataset_samples USING btree (dataset_id);

-- Name: ix_datasets_category; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_datasets_category ON public.datasets USING btree (category);

-- Name: ix_datasets_user_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_datasets_user_id ON public.datasets USING btree (user_id);

-- Name: ix_lab_runs_created_at; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_lab_runs_created_at ON public.lab_runs USING btree (created_at);

-- Name: ix_lab_runs_user_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_lab_runs_user_id ON public.lab_runs USING btree (user_id);

-- Name: ix_learning_modules_topic; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_learning_modules_topic ON public.learning_modules USING btree (topic);

-- Name: ix_lesson_progress_lesson_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_lesson_progress_lesson_id ON public.lesson_progress USING btree (lesson_id);

-- Name: ix_lesson_progress_user_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_lesson_progress_user_id ON public.lesson_progress USING btree (user_id);

-- Name: ix_lessons_module_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_lessons_module_id ON public.lessons USING btree (module_id);

-- Name: ix_model_lang_version; Type: INDEX; Schema: public; Owner: -

CREATE UNIQUE INDEX ix_model_lang_version ON public.model_versions USING btree (lang, version);

-- Name: ix_model_versions_lang; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_model_versions_lang ON public.model_versions USING btree (lang);

-- Name: ix_ocr_jobs_created_at; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_ocr_jobs_created_at ON public.ocr_jobs USING btree (created_at);

-- Name: ix_ocr_jobs_user_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_ocr_jobs_user_id ON public.ocr_jobs USING btree (user_id);

-- Name: ix_password_resets_token_hash; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_password_resets_token_hash ON public.password_resets USING btree (token_hash);

-- Name: ix_password_resets_user_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_password_resets_user_id ON public.password_resets USING btree (user_id);

-- Name: ix_quiz_attempts_quiz_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_quiz_attempts_quiz_id ON public.quiz_attempts USING btree (quiz_id);

-- Name: ix_quiz_attempts_user_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_quiz_attempts_user_id ON public.quiz_attempts USING btree (user_id);

-- Name: ix_quiz_questions_quiz_id; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_quiz_questions_quiz_id ON public.quiz_questions USING btree (quiz_id);

-- Name: ix_training_samples_lang; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_training_samples_lang ON public.training_samples USING btree (lang);

-- Name: ix_training_samples_status; Type: INDEX; Schema: public; Owner: -

CREATE INDEX ix_training_samples_status ON public.training_samples USING btree (status);

-- Name: ix_users_email; Type: INDEX; Schema: public; Owner: -

CREATE UNIQUE INDEX ix_users_email ON public.users USING btree (email);

-- Name: ix_users_username; Type: INDEX; Schema: public; Owner: -

CREATE UNIQUE INDEX ix_users_username ON public.users USING btree (username);

-- Name: uq_dataset_model_version; Type: INDEX; Schema: public; Owner: -

CREATE UNIQUE INDEX uq_dataset_model_version ON public.dataset_models USING btree (dataset_id, version);

-- Name: uq_dataset_user_name; Type: INDEX; Schema: public; Owner: -

CREATE UNIQUE INDEX uq_dataset_user_name ON public.datasets USING btree (user_id, name);

-- Name: uq_one_active_run_per_lang; Type: INDEX; Schema: public; Owner: -

CREATE UNIQUE INDEX uq_one_active_run_per_lang ON public.training_runs USING btree (lang) WHERE ((status)::text = ANY ((ARRAY['queued'::character varying, 'running'::character varying])::text[]));

-- Name: uq_one_training_per_dataset; Type: INDEX; Schema: public; Owner: -

CREATE UNIQUE INDEX uq_one_training_per_dataset ON public.dataset_models USING btree (dataset_id) WHERE ((status)::text = ANY ((ARRAY['queued'::character varying, 'training'::character varying])::text[]));

-- Name: uq_progress_user_lesson; Type: INDEX; Schema: public; Owner: -

CREATE UNIQUE INDEX uq_progress_user_lesson ON public.lesson_progress USING btree (user_id, lesson_id);

-- Name: api_keys api_keys_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.api_keys
    ADD CONSTRAINT api_keys_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE CASCADE;

-- Name: api_request_logs api_request_logs_api_key_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.api_request_logs
    ADD CONSTRAINT api_request_logs_api_key_id_fkey FOREIGN KEY (api_key_id) REFERENCES public.api_keys(id) ON DELETE SET NULL;

-- Name: api_request_logs api_request_logs_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.api_request_logs
    ADD CONSTRAINT api_request_logs_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE SET NULL;

-- Name: dataset_models dataset_models_dataset_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.dataset_models
    ADD CONSTRAINT dataset_models_dataset_id_fkey FOREIGN KEY (dataset_id) REFERENCES public.datasets(id) ON DELETE CASCADE;

-- Name: dataset_samples dataset_samples_dataset_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.dataset_samples
    ADD CONSTRAINT dataset_samples_dataset_id_fkey FOREIGN KEY (dataset_id) REFERENCES public.datasets(id) ON DELETE CASCADE;

-- Name: datasets datasets_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.datasets
    ADD CONSTRAINT datasets_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE CASCADE;

-- Name: lab_runs lab_runs_dataset_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.lab_runs
    ADD CONSTRAINT lab_runs_dataset_id_fkey FOREIGN KEY (dataset_id) REFERENCES public.datasets(id) ON DELETE SET NULL;

-- Name: lab_runs lab_runs_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.lab_runs
    ADD CONSTRAINT lab_runs_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE CASCADE;

-- Name: lesson_progress lesson_progress_lesson_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.lesson_progress
    ADD CONSTRAINT lesson_progress_lesson_id_fkey FOREIGN KEY (lesson_id) REFERENCES public.lessons(id) ON DELETE CASCADE;

-- Name: lesson_progress lesson_progress_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.lesson_progress
    ADD CONSTRAINT lesson_progress_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE CASCADE;

-- Name: lessons lessons_module_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.lessons
    ADD CONSTRAINT lessons_module_id_fkey FOREIGN KEY (module_id) REFERENCES public.learning_modules(id) ON DELETE CASCADE;

-- Name: ocr_jobs ocr_jobs_api_key_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.ocr_jobs
    ADD CONSTRAINT ocr_jobs_api_key_id_fkey FOREIGN KEY (api_key_id) REFERENCES public.api_keys(id) ON DELETE SET NULL;

-- Name: ocr_jobs ocr_jobs_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.ocr_jobs
    ADD CONSTRAINT ocr_jobs_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE CASCADE;

-- Name: password_resets password_resets_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.password_resets
    ADD CONSTRAINT password_resets_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE CASCADE;

-- Name: quiz_attempts quiz_attempts_api_key_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.quiz_attempts
    ADD CONSTRAINT quiz_attempts_api_key_id_fkey FOREIGN KEY (api_key_id) REFERENCES public.api_keys(id) ON DELETE SET NULL;

-- Name: quiz_attempts quiz_attempts_quiz_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.quiz_attempts
    ADD CONSTRAINT quiz_attempts_quiz_id_fkey FOREIGN KEY (quiz_id) REFERENCES public.quizzes(id) ON DELETE CASCADE;

-- Name: quiz_attempts quiz_attempts_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.quiz_attempts
    ADD CONSTRAINT quiz_attempts_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE CASCADE;

-- Name: quiz_questions quiz_questions_quiz_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.quiz_questions
    ADD CONSTRAINT quiz_questions_quiz_id_fkey FOREIGN KEY (quiz_id) REFERENCES public.quizzes(id) ON DELETE CASCADE;

-- Name: quizzes quizzes_lesson_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.quizzes
    ADD CONSTRAINT quizzes_lesson_id_fkey FOREIGN KEY (lesson_id) REFERENCES public.lessons(id) ON DELETE CASCADE;

-- Name: training_runs training_runs_requested_by_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.training_runs
    ADD CONSTRAINT training_runs_requested_by_fkey FOREIGN KEY (requested_by) REFERENCES public.users(id) ON DELETE SET NULL;

-- Name: training_runs training_runs_version_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.training_runs
    ADD CONSTRAINT training_runs_version_id_fkey FOREIGN KEY (version_id) REFERENCES public.model_versions(id) ON DELETE SET NULL;

-- Name: training_samples training_samples_api_key_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.training_samples
    ADD CONSTRAINT training_samples_api_key_id_fkey FOREIGN KEY (api_key_id) REFERENCES public.api_keys(id) ON DELETE SET NULL;

-- Name: training_samples training_samples_used_in_version_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.training_samples
    ADD CONSTRAINT training_samples_used_in_version_id_fkey FOREIGN KEY (used_in_version_id) REFERENCES public.model_versions(id) ON DELETE SET NULL;

-- Name: training_samples training_samples_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -

ALTER TABLE ONLY public.training_samples
    ADD CONSTRAINT training_samples_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE SET NULL;

\unrestrict muZFe1jw8E0gPiPSSD5bemguUHjd5XlPB7NARA5clI1O5SAEeaTSVchGFwVEHrr

